#include "PackHotkeys.h"

#include "FGCharacterPlayer.h"
#include "PackHotkeysComponent.h"

#define LOCTEXT_NAMESPACE "FPackHotkeysModule"

DEFINE_LOG_CATEGORY( LogPackHotkeys );

void FPackHotkeysModule::StartupModule()
{
	InputInitializedHandle = AFGCharacterPlayer::OnPlayerInputInitialized.AddStatic( &FPackHotkeysModule::HandlePlayerInputInitialized );
}

void FPackHotkeysModule::ShutdownModule()
{
	if( InputInitializedHandle.IsValid() )
	{
		AFGCharacterPlayer::OnPlayerInputInitialized.Remove( InputInitializedHandle );
		InputInitializedHandle.Reset();
	}
}

void FPackHotkeysModule::HandlePlayerInputInitialized( AFGCharacterPlayer* Character, UInputComponent* InputComponent )
{
	// Only the machine that owns this character has any business binding keys for it.
	if( !IsValid( Character ) || !Character->IsLocallyControlled() )
	{
		return;
	}

	UPackHotkeysComponent* Component = Character->FindComponentByClass< UPackHotkeysComponent >();
	if( !IsValid( Component ) )
	{
		Component = NewObject< UPackHotkeysComponent >( Character, UPackHotkeysComponent::StaticClass(), TEXT( "PackHotkeysComponent" ) );
		Component->RegisterComponent();
	}

	Component->SetupPlayerInput( InputComponent );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FPackHotkeysModule, PackHotkeys )
