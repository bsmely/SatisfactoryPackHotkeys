#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN( LogPackHotkeys, Log, All );

class AFGCharacterPlayer;
class UInputComponent;

class FPackHotkeysModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Called by the game every time a local player character finishes setting up its input component. */
	static void HandlePlayerInputInitialized( AFGCharacterPlayer* Character, UInputComponent* InputComponent );

	FDelegateHandle InputInitializedHandle;
};
