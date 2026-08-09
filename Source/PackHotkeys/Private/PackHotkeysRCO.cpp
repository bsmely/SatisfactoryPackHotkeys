#include "PackHotkeysRCO.h"

#include "PackHotkeys.h"
#include "PackHotkeysLibrary.h"

#include "FGCharacterPlayer.h"
#include "Net/UnrealNetwork.h"

void UPackHotkeysRCO::GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( UPackHotkeysRCO, bDummy );
}

bool UPackHotkeysRCO::Server_MoveIntoBackSlot_Validate( AFGCharacterPlayer* Character, int32 SourceIndex )
{
	return SourceIndex >= 0;
}

void UPackHotkeysRCO::Server_MoveIntoBackSlot_Implementation( AFGCharacterPlayer* Character, int32 SourceIndex )
{
	// A client may only move its own items around.
	if( !IsValid( Character ) || Character != GetOwnerPlayerCharacter() )
	{
		UE_LOG( LogPackHotkeys, Warning, TEXT( "Server_MoveIntoBackSlot called with a character the caller does not own" ) );
		return;
	}

	UPackHotkeysLibrary::MoveIntoBackSlot( Character, SourceIndex );
}

bool UPackHotkeysRCO::Server_MoveOutOfBackSlot_Validate( AFGCharacterPlayer* Character, int32 BackSlotIndex )
{
	return BackSlotIndex >= 0;
}

void UPackHotkeysRCO::Server_MoveOutOfBackSlot_Implementation( AFGCharacterPlayer* Character, int32 BackSlotIndex )
{
	if( !IsValid( Character ) || Character != GetOwnerPlayerCharacter() )
	{
		UE_LOG( LogPackHotkeys, Warning, TEXT( "Server_MoveOutOfBackSlot called with a character the caller does not own" ) );
		return;
	}

	UPackHotkeysLibrary::MoveOutOfBackSlot( Character, BackSlotIndex );
}
