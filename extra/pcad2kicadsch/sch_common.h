#ifndef SCH_COMMON_H_
#define SCH_COMMON_H_

#include <wx/wx.h>

#include <pcad2kicad_common.h>

namespace PCAD2KICAD {

void EscapeTextQuotes( wxString& aStr );
wxString GetJustifyString( const TTEXTVALUE* aValue );
void MirrorJustify( TTEXT_JUSTIFY* aJustify );
void RotateJustify180( TTEXT_JUSTIFY* aJustify );
int GetPenSizeForBold( int aTextSize );

} // namespace PCAD2KICAD

#endif // SCH_COMMON_H_
