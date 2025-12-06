#pragma once

#include <colors.hpp>
#include <palladium>

constexpr int elms_onscreen = 12;

class Cursor {
 public:
  Cursor(PD::fvec2 list_start, float entry_height) {
    pListStart = list_start;
    pEntryHeight = entry_height;
    pCursor.From(pListStart).To(pListStart).As(pCursor.Linear).In(0.5f);
  }
  ~Cursor() = default;

  PD_SHARED(Cursor);

  int GetIndex() const { return pIndex; }

  void SetIndex(int idx) {
    pIndex = idx;
    UpdatePos();
  }

  void UpdatePos() {
    pCursor.From(pCursor)
        .To(PD::fvec2(pListStart.x, pListStart.y + pEntryHeight * pIndex))
        .In(0.1f)
        .As(pCursor.EaseInOutSine);
  }

  void Update(float delta) { pCursor.Update(delta); }

  Cursor& operator++(int) {
    pIndex++;
    UpdatePos();
    return *this;
  }

  Cursor& operator--(int) {
    pIndex--;
    UpdatePos();
    return *this;
  }

  Cursor& operator+=(int num) {
    pIndex += num;
    UpdatePos();
    return *this;
  }

  Cursor& operator-=(int num) {
    pIndex -= num;
    UpdatePos();
    return *this;
  }

  operator PD::fvec2() const { return pCursor.Get(); }

  PD::Tween<PD::fvec2> pCursor;
  PD::fvec2 pListStart;
  PD::fvec2 pCursorSize;
  float pEntryHeight;
  int pIndex = 0;
};