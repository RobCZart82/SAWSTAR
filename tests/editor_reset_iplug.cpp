// SPDX-License-Identifier: MIT
// Verify the reset hook against the actual pinned SPSC queue when available.
#include "IPlugQueue.h"
#include <cstdlib>
using IMidiMsg=int;
struct Adapter {
  iplug::IPlugQueue<int> mMidiMsgsFromEditor{1024};
  bool TakeMidiMsgFromEditorOverflow(){return false;}
#include "editor_reset_hook.inc"
};
int main(){Adapter a;for(int i=0;i<1024;++i)if(!a.mMidiMsgsFromEditor.Push(i))return 1;a.DiscardPendingMidiFromEditor();if(!a.mMidiMsgsFromEditor.WasEmpty())return 2;a.mMidiMsgsFromEditor.Push(70);int n=0;return !a.mMidiMsgsFromEditor.Pop(n)||n!=70;}
