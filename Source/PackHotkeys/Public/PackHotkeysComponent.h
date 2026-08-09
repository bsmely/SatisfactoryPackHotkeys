#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "PackHotkeysComponent.generated.h"

class AFGEquipment;
class UFGInputMappingContext;
class UInputAction;
class UInputComponent;

/**
 * Added at runtime to the local player character. Owns the mod's input actions
 * and turns key presses into back equipment slot changes.
 */
UCLASS()
class PACKHOTKEYS_API UPackHotkeysComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPackHotkeysComponent();

	/** Binds the mod's actions onto the character's freshly created input component. */
	void SetupPlayerInput( UInputComponent* InputComponent );

protected:
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

private:
	void CreateInputActions();
	void AddMappingContext();
	void RemoveMappingContext();

	void OnJetPackPressed();
	void OnHoverPackPressed();
	void OnParachutePressed();

	/** Equips the first item in reach whose equipment class derives from EquipmentClass. */
	void EquipPack( TSubclassOf< AFGEquipment > EquipmentClass );

	/** True while the player is typing (chat, sign text, search fields), so we keep our hands off the keys. */
	static bool IsTypingIntoWidget();

	UPROPERTY( Transient )
	TObjectPtr< UInputAction > JetPackAction;

	UPROPERTY( Transient )
	TObjectPtr< UInputAction > HoverPackAction;

	UPROPERTY( Transient )
	TObjectPtr< UInputAction > ParachuteAction;

	UPROPERTY( Transient )
	TObjectPtr< UFGInputMappingContext > MappingContext;
};
