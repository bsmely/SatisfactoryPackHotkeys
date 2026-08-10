#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PackHotkeysSettings.generated.h"

/**
 * Mod configuration, stored in the [/Script/PackHotkeys.PackHotkeysSettings] section of Game.ini.
 *
 * The keys themselves are not here - they are regular game keybindings, rebindable under
 * Options > Keybindings in the "Pack Hotkeys" category.
 */
UCLASS( Config = Game, DefaultConfig, meta = ( DisplayName = "Pack Hotkeys" ) )
class PACKHOTKEYS_API UPackHotkeysSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPackHotkeysSettings();

	/**
	 * When true, pressing the key of the pack that is already equipped puts it back into the inventory.
	 * When false (default), pressing it again does nothing.
	 */
	UPROPERTY( Config, EditAnywhere, Category = "Behaviour" )
	bool bUnequipWhenAlreadyEquipped;

	static const UPackHotkeysSettings* Get() { return GetDefault< UPackHotkeysSettings >(); }
};
