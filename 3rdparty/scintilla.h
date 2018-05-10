#define SCI_NAMESPACE
#include "scintilla/include/Platform.h"

#include "scintilla/include/ILexer.h"
#include "scintilla/include/Scintilla.h"

#ifdef SCI_LEXER
#include"scintilla/lexlib/include.h"
#endif

#include"scintilla/lexlib/PropSetSimple.h"
#include"scintilla/lexlib/StringCopy.h"

#ifdef SCI_LEXER
#include"scintilla/src/LexerModule.h"
#include"scintilla/src/Catalogue.h"
#endif

#include"scintilla/src/SplitVector.h"
#include"scintilla/src/Partitioning.h"
#include"scintilla/src/RunStyles.h"
#include"scintilla/src/ContractionState.h"
#include"scintilla/src/CellBuffer.h"
#include"scintilla/src/CallTip.h"
#include"scintilla/src/KeyMap.h"
#include"scintilla/src/Indicator.h"
#include"scintilla/src/XPM.h"
#include"scintilla/src/LineMarker.h"
#include"scintilla/src/Style.h"
#include"scintilla/src/ViewStyle.h"
#include"scintilla/src/CharClassify.h"
#include"scintilla/src/Decoration.h"
#include"scintilla/src/CaseFolder.h"
#include"scintilla/src/Document.h"
#include"scintilla/src/Selection.h"
#include"scintilla/src/PositionCache.h"
#include"scintilla/src/EditModel.h"
#include"scintilla/src/MarginView.h"
#include"scintilla/src/EditView.h"
#include"scintilla/src/Editor.h"
#include"scintilla/src/AutoComplete.h"
#include"scintilla/src/ScintillaBase.h"
#include "scintilla/src/UniConversion.h"
#include "scintilla/src/CaseConvert.h"