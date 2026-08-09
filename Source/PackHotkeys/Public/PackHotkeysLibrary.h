#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "PackHotkeysLibrary.generated.h"

class AFGCharacterPlayer;
class AFGEquipment;
class UFGInventoryComponent;

/**
 * Inventory plumbing shared between the local input handling and the server side of the RCO.
 */
UCLASS()
class PACKHOTKEYS_API UPackHotkeysLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Finds the first slot holding an item whose equipment class derives from EquipmentClass.
	 * Works on any inventory, including an equipment slot component.
	 * @return the slot index, or INDEX_NONE if the inventory holds no such item.
	 */
	UFUNCTION( BlueprintPure, Category = "Pack Hotkeys" )
	static int32 FindIndexOfEquipmentClass( UFGInventoryComponent* Inventory, TSubclassOf< AFGEquipment > EquipmentClass );

	/** @return the first empty slot index, or INDEX_NONE if the inventory is full. */
	UFUNCTION( BlueprintPure, Category = "Pack Hotkeys" )
	static int32 FindFirstEmptyIndex( UFGInventoryComponent* Inventory );

	/**
	 * Moves the item at SourceIndex of the player's inventory into the back equipment slot,
	 * swapping out whatever is currently in there.
	 * @note Server only - inventory moves are authority-side in Satisfactory.
	 */
	static bool MoveIntoBackSlot( AFGCharacterPlayer* Character, int32 SourceIndex );

	/**
	 * Moves the item at BackSlotIndex of the back equipment slot into the first free inventory slot.
	 * @note Server only.
	 */
	static bool MoveOutOfBackSlot( AFGCharacterPlayer* Character, int32 BackSlotIndex );
};
