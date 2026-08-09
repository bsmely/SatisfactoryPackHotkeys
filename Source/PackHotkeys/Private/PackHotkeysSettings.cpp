#include "PackHotkeysSettings.h"

UPackHotkeysSettings::UPackHotkeysSettings()
	: JetPackKey( EKeys::F1 )
	, HoverPackKey( EKeys::F2 )
	, ParachuteKey( EKeys::F )
	, bUnequipWhenAlreadyEquipped( false )
	, MappingContextPriority( 1000 )
{
}
