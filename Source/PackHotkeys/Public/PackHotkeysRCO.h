#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "PackHotkeysRCO.generated.h"

class AFGCharacterPlayer;

/**
 * Lets a client ask the server to move packs in and out of the back equipment slot,
 * since inventory moves are authority-only.
 */
UCLASS()
class PACKHOTKEYS_API UPackHotkeysRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;

	UFUNCTION( Server, Reliable, WithValidation )
	void Server_MoveIntoBackSlot( AFGCharacterPlayer* Character, int32 SourceIndex );

	UFUNCTION( Server, Reliable, WithValidation )
	void Server_MoveOutOfBackSlot( AFGCharacterPlayer* Character, int32 BackSlotIndex );

	/** Unused, but an RCO needs at least one replicated property to replicate at all. */
	UPROPERTY( Replicated )
	bool bDummy = true;
};
