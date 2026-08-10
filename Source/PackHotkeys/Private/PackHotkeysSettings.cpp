#include "PackHotkeysSettings.h"

UPackHotkeysSettings::UPackHotkeysSettings()
	: JetPackKey( EKeys::F1 )
	, HoverPackKey( EKeys::F2 )
	, ParachuteKey( EKeys::F3 )
	, bUnequipWhenAlreadyEquipped( false )
	, MappingContextPriority( 1000 )
{
}
