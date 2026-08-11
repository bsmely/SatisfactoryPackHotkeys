#include "PackHotkeysComponent.h"

#include "PackHotkeys.h"
#include "PackHotkeysLibrary.h"
#include "PackHotkeysRCO.h"
#include "PackHotkeysSettings.h"

#include "EnhancedInputComponent.h"
#include "Equipment/FGEquipment.h"
#include "Equipment/FGHoverPack.h"
#include "Equipment/FGJetPack.h"
#include "Equipment/FGParachute.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGInventoryComponentEquipment.h"
#include "FGPlayerController.h"
#include "InputAction.h"

namespace PackHotkeysActions
{
	const TCHAR* TogglePack = TEXT( "/PackHotkeys/Inputs/IA_TogglePack.IA_TogglePack" );
	const TCHAR* Parachute = TEXT( "/PackHotkeys/Inputs/IA_EquipParachute.IA_EquipParachute" );
}

UPackHotkeysComponent::UPackHotkeysComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPackHotkeysComponent::SetupPlayerInput( UInputComponent* InputComponent )
{
	UEnhancedInputComponent* EnhancedInput = Cast< UEnhancedInputComponent >( InputComponent );
	if( !EnhancedInput )
	{
		UE_LOG( LogPackHotkeys, Error, TEXT( "Player input component is not an UEnhancedInputComponent, cannot bind keys" ) );
		return;
	}

	// The mapping context that carries these actions is a child of MC_PlayerActions,
	// so the game binds and unbinds it for us - we only have to listen for the actions.
	auto BindAction = [ this, EnhancedInput ]( const TCHAR* AssetPath, void ( UPackHotkeysComponent::*Handler )() )
	{
		if( UInputAction* Action = LoadObject< UInputAction >( nullptr, AssetPath ) )
		{
			EnhancedInput->BindAction( Action, ETriggerEvent::Started, this, Handler );
		}
		else
		{
			UE_LOG( LogPackHotkeys, Error, TEXT( "Could not load input action %s" ), AssetPath );
		}
	};

	BindAction( PackHotkeysActions::TogglePack, &UPackHotkeysComponent::OnTogglePackPressed );
	BindAction( PackHotkeysActions::Parachute, &UPackHotkeysComponent::OnParachutePressed );

	// Make sure the RCO exists on this side before the first key press needs it.
	const APawn* Pawn = Cast< APawn >( GetOwner() );
	if( AFGPlayerController* Controller = Pawn ? Cast< AFGPlayerController >( Pawn->GetController() ) : nullptr )
	{
		Controller->RegisterRemoteCallObjectClass( UPackHotkeysRCO::StaticClass() );
	}
}

void UPackHotkeysComponent::OnTogglePackPressed()
{
	const AFGCharacterPlayer* Character = Cast< AFGCharacterPlayer >( GetOwner() );
	if( !IsValid( Character ) )
	{
		return;
	}

	const AFGEquipment* Equipped = Character->GetEquipmentInSlot( EEquipmentSlot::ES_BACK );

	// Wearing one of the two packs: swap to the other one, and stay put if we don't have it.
	if( Equipped && Equipped->IsA( AFGJetPack::StaticClass() ) )
	{
		EquipPack( AFGHoverPack::StaticClass() );
		return;
	}
	if( Equipped && Equipped->IsA( AFGHoverPack::StaticClass() ) )
	{
		EquipPack( AFGJetPack::StaticClass() );
		return;
	}

	// Nothing (or the parachute) on our back: put a pack on, whichever we happen to carry.
	if( !EquipPack( AFGJetPack::StaticClass() ) )
	{
		EquipPack( AFGHoverPack::StaticClass() );
	}
}

void UPackHotkeysComponent::OnParachutePressed()
{
	EquipPack( AFGParachute::StaticClass() );
}

bool UPackHotkeysComponent::EquipPack( TSubclassOf< AFGEquipment > EquipmentClass )
{
	AFGCharacterPlayer* Character = Cast< AFGCharacterPlayer >( GetOwner() );
	if( !IsValid( Character ) )
	{
		return false;
	}

	AFGPlayerController* Controller = Cast< AFGPlayerController >( Character->GetController() );

	UFGInventoryComponentEquipment* BackSlot = Character->GetEquipmentSlot( EEquipmentSlot::ES_BACK );
	if( !IsValid( BackSlot ) )
	{
		return false;
	}

	const UPackHotkeysSettings* Settings = UPackHotkeysSettings::Get();

	// Already wearing this kind of pack?
	if( const AFGEquipment* Equipped = Character->GetEquipmentInSlot( EEquipmentSlot::ES_BACK ) )
	{
		if( Equipped->IsA( EquipmentClass ) )
		{
			if( !Settings->bUnequipWhenAlreadyEquipped )
			{
				return true;
			}

			const int32 ActiveIndex = BackSlot->GetActiveIndex();
			if( Character->HasAuthority() )
			{
				UPackHotkeysLibrary::MoveOutOfBackSlot( Character, ActiveIndex );
			}
			else if( UPackHotkeysRCO* RCO = Controller ? Controller->GetRemoteCallObjectOfClass< UPackHotkeysRCO >() : nullptr )
			{
				RCO->Server_MoveOutOfBackSlot( Character, ActiveIndex );
			}
			return true;
		}
	}

	// The back slot itself can hold more than one item - if the pack is already in there, just make it active.
	const int32 SlotIndex = UPackHotkeysLibrary::FindIndexOfEquipmentClass( BackSlot, EquipmentClass );
	if( SlotIndex != INDEX_NONE )
	{
		// Server RPC on a component the local player owns, so this is fine to call from the client.
		BackSlot->Server_SetActiveEquipmentIndex( SlotIndex );
		return true;
	}

	// Otherwise fetch it out of the player inventory.
	UFGInventoryComponent* Inventory = Character->GetInventory();
	const int32 SourceIndex = UPackHotkeysLibrary::FindIndexOfEquipmentClass( Inventory, EquipmentClass );
	if( SourceIndex == INDEX_NONE )
	{
		UE_LOG( LogPackHotkeys, Verbose, TEXT( "No item of class %s in the inventory" ), *GetNameSafe( EquipmentClass ) );
		return false;
	}

	if( Character->HasAuthority() )
	{
		UPackHotkeysLibrary::MoveIntoBackSlot( Character, SourceIndex );
	}
	else if( UPackHotkeysRCO* RCO = Controller ? Controller->GetRemoteCallObjectOfClass< UPackHotkeysRCO >() : nullptr )
	{
		RCO->Server_MoveIntoBackSlot( Character, SourceIndex );
	}
	else
	{
		UE_LOG( LogPackHotkeys, Warning, TEXT( "No remote call object available - is the mod installed on the server?" ) );
		return false;
	}

	return true;
}
