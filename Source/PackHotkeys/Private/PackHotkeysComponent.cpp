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
	const TCHAR* JetPack = TEXT( "/PackHotkeys/Inputs/IA_EquipJetPack.IA_EquipJetPack" );
	const TCHAR* HoverPack = TEXT( "/PackHotkeys/Inputs/IA_EquipHoverPack.IA_EquipHoverPack" );
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

	BindAction( PackHotkeysActions::JetPack, &UPackHotkeysComponent::OnJetPackPressed );
	BindAction( PackHotkeysActions::HoverPack, &UPackHotkeysComponent::OnHoverPackPressed );
	BindAction( PackHotkeysActions::Parachute, &UPackHotkeysComponent::OnParachutePressed );

	// Make sure the RCO exists on this side before the first key press needs it.
	const APawn* Pawn = Cast< APawn >( GetOwner() );
	if( AFGPlayerController* Controller = Pawn ? Cast< AFGPlayerController >( Pawn->GetController() ) : nullptr )
	{
		Controller->RegisterRemoteCallObjectClass( UPackHotkeysRCO::StaticClass() );
	}
}

void UPackHotkeysComponent::OnJetPackPressed()
{
	EquipPack( AFGJetPack::StaticClass() );
}

void UPackHotkeysComponent::OnHoverPackPressed()
{
	EquipPack( AFGHoverPack::StaticClass() );
}

void UPackHotkeysComponent::OnParachutePressed()
{
	EquipPack( AFGParachute::StaticClass() );
}

void UPackHotkeysComponent::EquipPack( TSubclassOf< AFGEquipment > EquipmentClass )
{
	AFGCharacterPlayer* Character = Cast< AFGCharacterPlayer >( GetOwner() );
	if( !IsValid( Character ) )
	{
		return;
	}

	AFGPlayerController* Controller = Cast< AFGPlayerController >( Character->GetController() );

	UFGInventoryComponentEquipment* BackSlot = Character->GetEquipmentSlot( EEquipmentSlot::ES_BACK );
	if( !IsValid( BackSlot ) )
	{
		return;
	}

	const UPackHotkeysSettings* Settings = UPackHotkeysSettings::Get();

	// Already wearing this kind of pack?
	if( const AFGEquipment* Equipped = Character->GetEquipmentInSlot( EEquipmentSlot::ES_BACK ) )
	{
		if( Equipped->IsA( EquipmentClass ) )
		{
			if( !Settings->bUnequipWhenAlreadyEquipped )
			{
				return;
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
			return;
		}
	}

	// The back slot itself can hold more than one item - if the pack is already in there, just make it active.
	const int32 SlotIndex = UPackHotkeysLibrary::FindIndexOfEquipmentClass( BackSlot, EquipmentClass );
	if( SlotIndex != INDEX_NONE )
	{
		// Server RPC on a component the local player owns, so this is fine to call from the client.
		BackSlot->Server_SetActiveEquipmentIndex( SlotIndex );
		return;
	}

	// Otherwise fetch it out of the player inventory.
	UFGInventoryComponent* Inventory = Character->GetInventory();
	const int32 SourceIndex = UPackHotkeysLibrary::FindIndexOfEquipmentClass( Inventory, EquipmentClass );
	if( SourceIndex == INDEX_NONE )
	{
		UE_LOG( LogPackHotkeys, Verbose, TEXT( "No item of class %s in the inventory" ), *GetNameSafe( EquipmentClass ) );
		return;
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
	}
}
