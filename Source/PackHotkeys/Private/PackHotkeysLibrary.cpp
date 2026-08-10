#include "PackHotkeysLibrary.h"

#include "PackHotkeys.h"

#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGInventoryComponentEquipment.h"
#include "FGInventoryLibrary.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "Resources/FGItemDescriptor.h"

int32 UPackHotkeysLibrary::FindIndexOfEquipmentClass( UFGInventoryComponent* Inventory, TSubclassOf< AFGEquipment > EquipmentClass )
{
	if( !IsValid( Inventory ) || !IsValid( EquipmentClass ) )
	{
		return INDEX_NONE;
	}

	const int32 NumSlots = Inventory->GetSizeLinear();
	for( int32 Index = 0; Index < NumSlots; ++Index )
	{
		FInventoryStack Stack;
		if( !Inventory->GetStackFromIndex( Index, Stack ) || !Stack.HasItems() )
		{
			continue;
		}

		const TSubclassOf< UFGItemDescriptor > ItemClass = Stack.Item.GetItemClass();
		if( !IsValid( ItemClass ) )
		{
			continue;
		}

		// GetEquipmentClass asserts if handed a descriptor that is not an equipment descriptor,
		// so ordinary items (ore, plates, ...) have to be filtered out before calling it.
		if( !ItemClass->IsChildOf( UFGEquipmentDescriptor::StaticClass() ) )
		{
			continue;
		}

		const TSubclassOf< AFGEquipment > ItemEquipmentClass = UFGEquipmentDescriptor::GetEquipmentClass( ItemClass );
		if( IsValid( ItemEquipmentClass ) && ItemEquipmentClass->IsChildOf( EquipmentClass ) )
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 UPackHotkeysLibrary::FindFirstEmptyIndex( UFGInventoryComponent* Inventory )
{
	if( !IsValid( Inventory ) )
	{
		return INDEX_NONE;
	}

	const int32 NumSlots = Inventory->GetSizeLinear();
	for( int32 Index = 0; Index < NumSlots; ++Index )
	{
		FInventoryStack Stack;
		if( !Inventory->GetStackFromIndex( Index, Stack ) || !Stack.HasItems() )
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

bool UPackHotkeysLibrary::MoveIntoBackSlot( AFGCharacterPlayer* Character, int32 SourceIndex )
{
	if( !IsValid( Character ) || !Character->HasAuthority() )
	{
		return false;
	}

	UFGInventoryComponent* Inventory = Character->GetInventory();
	UFGInventoryComponentEquipment* BackSlot = Character->GetEquipmentSlot( EEquipmentSlot::ES_BACK );
	if( !IsValid( Inventory ) || !IsValid( BackSlot ) || !Inventory->IsValidIndex( SourceIndex ) )
	{
		return false;
	}

	// Prefer a free slot; otherwise target the active one so MoveInventoryItem swaps the packs over.
	int32 DestinationIndex = FindFirstEmptyIndex( BackSlot );
	if( DestinationIndex == INDEX_NONE )
	{
		DestinationIndex = FMath::Max( BackSlot->GetActiveIndex(), 0 );
	}

	const bool bMoved = UFGInventoryLibrary::MoveInventoryItem( Inventory, SourceIndex, BackSlot, DestinationIndex );
	if( bMoved )
	{
		BackSlot->SetActiveEquipmentIndex( DestinationIndex );
	}
	else
	{
		UE_LOG( LogPackHotkeys, Warning, TEXT( "Failed to move inventory slot %d into back slot %d" ), SourceIndex, DestinationIndex );
	}

	return bMoved;
}

bool UPackHotkeysLibrary::MoveOutOfBackSlot( AFGCharacterPlayer* Character, int32 BackSlotIndex )
{
	if( !IsValid( Character ) || !Character->HasAuthority() )
	{
		return false;
	}

	UFGInventoryComponent* Inventory = Character->GetInventory();
	UFGInventoryComponentEquipment* BackSlot = Character->GetEquipmentSlot( EEquipmentSlot::ES_BACK );
	if( !IsValid( Inventory ) || !IsValid( BackSlot ) || !BackSlot->IsValidIndex( BackSlotIndex ) )
	{
		return false;
	}

	const int32 DestinationIndex = FindFirstEmptyIndex( Inventory );
	if( DestinationIndex == INDEX_NONE )
	{
		UE_LOG( LogPackHotkeys, Warning, TEXT( "Inventory is full, cannot unequip the back item" ) );
		return false;
	}

	return UFGInventoryLibrary::MoveInventoryItem( BackSlot, BackSlotIndex, Inventory, DestinationIndex );
}
