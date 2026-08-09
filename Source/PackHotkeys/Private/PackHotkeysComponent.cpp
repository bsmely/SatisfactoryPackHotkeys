#include "PackHotkeysComponent.h"

#include "PackHotkeys.h"
#include "PackHotkeysLibrary.h"
#include "PackHotkeysRCO.h"
#include "PackHotkeysSettings.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Equipment/FGEquipment.h"
#include "Equipment/FGHoverPack.h"
#include "Equipment/FGJetPack.h"
#include "Equipment/FGParachute.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGInventoryComponentEquipment.h"
#include "FGPlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/FGInputMappingContext.h"
#include "InputAction.h"

#define LOCTEXT_NAMESPACE "PackHotkeys"

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

	CreateInputActions();
	AddMappingContext();

	EnhancedInput->BindAction( JetPackAction, ETriggerEvent::Started, this, &UPackHotkeysComponent::OnJetPackPressed );
	EnhancedInput->BindAction( HoverPackAction, ETriggerEvent::Started, this, &UPackHotkeysComponent::OnHoverPackPressed );
	EnhancedInput->BindAction( ParachuteAction, ETriggerEvent::Started, this, &UPackHotkeysComponent::OnParachutePressed );

	// Make sure the RCO exists on this side before the first key press needs it.
	const APawn* Pawn = Cast< APawn >( GetOwner() );
	if( AFGPlayerController* Controller = Pawn ? Cast< AFGPlayerController >( Pawn->GetController() ) : nullptr )
	{
		Controller->RegisterRemoteCallObjectClass( UPackHotkeysRCO::StaticClass() );
	}
}

void UPackHotkeysComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	RemoveMappingContext();

	Super::EndPlay( EndPlayReason );
}

void UPackHotkeysComponent::CreateInputActions()
{
	if( MappingContext )
	{
		return;
	}

	const UPackHotkeysSettings* Settings = UPackHotkeysSettings::Get();

	auto MakeAction = [ this ]( const TCHAR* Name, const FText& Description ) -> UInputAction*
	{
		UInputAction* Action = NewObject< UInputAction >( this, FName( Name ) );
		Action->ValueType = EInputActionValueType::Boolean;
		Action->ActionDescription = Description;
		// Never swallow the key - other mods and the base game may want it too.
		Action->bConsumeInput = false;
		return Action;
	};

	JetPackAction = MakeAction( TEXT( "IA_PackHotkeys_JetPack" ), LOCTEXT( "JetPackAction", "Equip Jetpack" ) );
	HoverPackAction = MakeAction( TEXT( "IA_PackHotkeys_HoverPack" ), LOCTEXT( "HoverPackAction", "Equip Hover Pack" ) );
	ParachuteAction = MakeAction( TEXT( "IA_PackHotkeys_Parachute" ), LOCTEXT( "ParachuteAction", "Equip Parachute" ) );

	MappingContext = NewObject< UFGInputMappingContext >( this, TEXT( "MC_PackHotkeys" ) );
	MappingContext->mDisplayName = LOCTEXT( "MappingContextName", "Pack Hotkeys" );
	MappingContext->mMenuPriority = 200.0f;

	MappingContext->MapKey( JetPackAction, Settings->JetPackKey );
	MappingContext->MapKey( HoverPackAction, Settings->HoverPackKey );
	MappingContext->MapKey( ParachuteAction, Settings->ParachuteKey );
}

void UPackHotkeysComponent::AddMappingContext()
{
	const APawn* Pawn = Cast< APawn >( GetOwner() );
	const APlayerController* Controller = Pawn ? Cast< APlayerController >( Pawn->GetController() ) : nullptr;
	const ULocalPlayer* LocalPlayer = Controller ? Controller->GetLocalPlayer() : nullptr;
	if( !LocalPlayer )
	{
		return;
	}

	if( UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
	{
		// The context is registered manually rather than through the parent context system,
		// because that one only picks up mapping contexts that exist as cooked assets.
		Subsystem->AddMappingContext( MappingContext, UPackHotkeysSettings::Get()->MappingContextPriority );
	}
}

void UPackHotkeysComponent::RemoveMappingContext()
{
	if( !MappingContext )
	{
		return;
	}

	const APawn* Pawn = Cast< APawn >( GetOwner() );
	const APlayerController* Controller = Pawn ? Cast< APlayerController >( Pawn->GetController() ) : nullptr;
	const ULocalPlayer* LocalPlayer = Controller ? Controller->GetLocalPlayer() : nullptr;
	if( !LocalPlayer )
	{
		return;
	}

	if( UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
	{
		Subsystem->RemoveMappingContext( MappingContext );
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
	if( IsTypingIntoWidget() )
	{
		return;
	}

	AFGCharacterPlayer* Character = Cast< AFGCharacterPlayer >( GetOwner() );
	if( !IsValid( Character ) )
	{
		return;
	}

	AFGPlayerController* Controller = Cast< AFGPlayerController >( Character->GetController() );
	if( IsValid( Controller ) && Controller->IsPauseMenuOpen() )
	{
		return;
	}

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

bool UPackHotkeysComponent::IsTypingIntoWidget()
{
	if( !FSlateApplication::IsInitialized() )
	{
		return false;
	}

	const TSharedPtr< SWidget > FocusedWidget = FSlateApplication::Get().GetKeyboardFocusedWidget();
	if( !FocusedWidget.IsValid() )
	{
		return false;
	}

	const FString WidgetType = FocusedWidget->GetTypeAsString();
	return WidgetType.Contains( TEXT( "EditableText" ) );
}

#undef LOCTEXT_NAMESPACE
