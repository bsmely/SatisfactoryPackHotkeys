#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "PackHotkeysComponent.generated.h"

class AFGEquipment;
class UInputAction;
class UInputComponent;

/**
 * Added at runtime to the local player character, binds the mod's input actions
 * and turns key presses into back equipment slot changes.
 *
 * The actions themselves live as assets under /PackHotkeys/Inputs so the game can
 * list them in Options > Keybindings; MC_PackHotkeys is a child context of
 * MC_PlayerActions, so the game applies and removes it for us.
 */
UCLASS()
class PACKHOTKEYS_API UPackHotkeysComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPackHotkeysComponent();

	/** Binds the mod's actions onto the character's freshly created input component. */
	void SetupPlayerInput( UInputComponent* InputComponent );

private:
	/** Equips the Jetpack, or swaps between Jetpack and Hover Pack if one of them is worn. */
	void OnTogglePackPressed();
	void OnParachutePressed();

	/**
	 * Equips the first item within reach whose equipment class derives from EquipmentClass.
	 * @return false if no such item is in the back slot or the player inventory.
	 */
	bool EquipPack( TSubclassOf< AFGEquipment > EquipmentClass );
};
