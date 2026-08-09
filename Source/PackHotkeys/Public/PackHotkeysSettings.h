#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "PackHotkeysSettings.generated.h"

/**
 * Mod configuration, stored in the [/Script/PackHotkeys.PackHotkeysSettings] section of Game.ini.
 * The mod has no Blueprint content, so it cannot use the in-game mod settings menu -
 * editing the ini file is the way to change these.
 */
UCLASS( Config = Game, DefaultConfig, meta = ( DisplayName = "Pack Hotkeys" ) )
class PACKHOTKEYS_API UPackHotkeysSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPackHotkeysSettings();

	/** Key that equips the Jetpack. */
	UPROPERTY( Config, EditAnywhere, Category = "Keys" )
	FKey JetPackKey;

	/** Key that equips the Hover Pack. */
	UPROPERTY( Config, EditAnywhere, Category = "Keys" )
	FKey HoverPackKey;

	/** Key that equips the Parachute. */
	UPROPERTY( Config, EditAnywhere, Category = "Keys" )
	FKey ParachuteKey;

	/**
	 * When true, pressing the key of the pack that is already equipped puts it back into the inventory.
	 * When false (default), pressing it again does nothing.
	 */
	UPROPERTY( Config, EditAnywhere, Category = "Behaviour" )
	bool bUnequipWhenAlreadyEquipped;

	/**
	 * Priority of the mod's mapping context. Higher wins over contexts that map the same key.
	 * Raise this if another mod or the base game steals one of the keys.
	 */
	UPROPERTY( Config, EditAnywhere, Category = "Behaviour" )
	int32 MappingContextPriority;

	static const UPackHotkeysSettings* Get() { return GetDefault< UPackHotkeysSettings >(); }
};
