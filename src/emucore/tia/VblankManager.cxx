//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2017 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

// #define TIA_VBLANK_MANAGER_DEBUG_LOG

#include "VblankManager.hxx"

enum Metrics: uInt32 {
  maxUnderscan                  = 10,
  maxVblankViolations           = 2,
  minStableVblankFrames         = 1
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
VblankManager::VblankManager()
  : myVblankLines(0),
    myMaxUnderscan(0),
    myYstart(0),
    myMode(VblankMode::floating)
{
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void VblankManager::reset()
{
  myVblank = false;
  myCurrentLine = 0;
  myVblankViolations = 0;
  myStableVblankFrames = 0;
  myVblankViolated = false;
  myLastVblankLines = 0;

  if (myMode != VblankMode::fixed) myMode = VblankMode::floating;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void VblankManager::start()
{
  myCurrentLine = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool VblankManager::nextLine(bool isGarbageFrame)
{
  myCurrentLine++;

  bool shouldTransition = myCurrentLine >= (myVblank ? myVblankLines : myVblankLines - Metrics::maxUnderscan);
  bool transition = false;

  switch (myMode) {
    case VblankMode::floating:

      if (shouldTransition) {
        if (!isGarbageFrame && myCurrentLine == myLastVblankLines)
          myStableVblankFrames++;
        else
          myStableVblankFrames = 0;

        myLastVblankLines = myCurrentLine;

#ifdef TIA_VBLANK_MANAGER_DEBUG_LOG
        (cout << "leaving vblank in floating mode, should transition: " << shouldTransition << "\n").flush();
#endif
        transition = true;
      }

      if (myStableVblankFrames >= Metrics::minStableVblankFrames) {
        myMode = VblankMode::locked;
        myVblankViolations = 0;
      }

      break;

    case VblankMode::locked:

      if (myCurrentLine == myLastVblankLines) {

        if (shouldTransition && !myVblankViolated)
          myVblankViolations = 0;
        else {
          if (!myVblankViolated) myVblankViolations++;
          myVblankViolated = true;
        }

#ifdef TIA_VBLANK_MANAGER_DEBUG_LOG
        (cout << "leaving vblank in locked mode, should transition: " << shouldTransition << "\n").flush();
#endif

        transition = true;
      } else if (shouldTransition){
        if (!myVblankViolated) myVblankViolations++;
        myVblankViolated = true;
      }

      if (myVblankViolations > Metrics::maxVblankViolations) {
        myMode = VblankMode::floating;
        myStableVblankFrames = 0;
      }

      break;

    case VblankMode::fixed:
      if (myCurrentLine > myYstart) transition = true;
      break;

  }

  return transition;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void VblankManager::setVblankLines(uInt32 vblankLines)
{
  myVblankLines = vblankLines;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void VblankManager::setYstart(uInt32 ystart)
{
  if (ystart == myYstart) return;

  myYstart = ystart;

  myMode = ystart ? VblankMode::fixed : VblankMode::floating;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void VblankManager::setVblank(bool vblank)
{
#ifdef TIA_VBLANK_MANAGER_DEBUG_LOG
  if (myVblank != vblank)
    (cout << "vblank " << myVblank << " -> " << vblank << ": state " << int(myState) << " @ " << myLineInState << "\n").flush();
#endif

  myVblank = vblank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TODO: implement this once the class is finalized
bool VblankManager::save(Serializer& out) const
{
  try
  {
    out.putString(name());

    // TODO - save instance variables
  }
  catch(...)
  {
    cerr << "ERROR: TIA_VblankManager::save" << endl;
    return false;
  }

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// TODO: implement this once the class is finalized
bool VblankManager::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    // TODO - load instance variables
  }
  catch(...)
  {
    cerr << "ERROR: TIA_VblankManager::load" << endl;
    return false;
  }

  return false;
}