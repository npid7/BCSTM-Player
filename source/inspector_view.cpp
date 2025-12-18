#include <ctrff.hpp>
#include <inspector_view.hpp>
#include <stages.hpp>

void Inspector::Update() {
  delta.Update();
  cursor.Update(delta.GetSeconds() * 1000.f);
  delta.Reset();

  for (int i = 0; i < 12; i++) {
    Top->Rect()
        .SetPos(PD::fvec2(0, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) == 0) ? gTheme.ListEven : gTheme.ListOdd);
    Top->Rect()
        .SetPos(PD::fvec2(200, 18 + 17 * i))
        .SetSize(PD::fvec2(200, 17))
        .SetColor(((i % 2) != 0) ? gTheme.ListEven : gTheme.ListOdd);
  }
  Top->Rect()
      .SetPos(cursor)
      .SetSize(PD::fvec2(400, 17))
      .SetColor(gTheme.Selector);
  for (int i = 0; i < int(pDL.size() > 12 ? 12 : pDL.size()); i++) {
    Top->Text(pDL.at(sp + i)->First)
        .SetPos(PD::fvec2(5, 18 + 17 * i))
        .SetColor(gTheme.Text);
    Top->Text(pDL.at(sp + i)->Second)
        .SetPos(PD::fvec2(205, 18 + 17 * i))
        .SetColor(gTheme.Text);
  }
  Top->Rect().SetColor(gTheme.Header).SetPos(0).SetSize(PD::fvec2(400, 18));
  Top->Text("BCSTM-Player -> File Inspector")
      .SetPos(PD::fvec2(5, 1))
      .SetColor(gTheme.Text);
  Top->Rect()
      .SetPos(PD::fvec2(0, 222))
      .SetSize(PD::fvec2(400, 18))
      .SetColor(gTheme.Footer);
  /** Only use Filename due to no space */
  Top->Text(std::filesystem::path(pPath).filename().string())
      .SetPos(PD::fvec2(5, 223))
      .SetColor(gTheme.Text);
  if (pDL.size() > 12) {
    float rect_h = (12.f / (float)pDL.size()) * 204.f;
    /** Make sure the rect is still visible */
    rect_h = std::clamp<float>(rect_h, 10.f, 204.f);
    float rect_pos =
        18.f + ((float)sp / (float)(pDL.size() - 12)) * (204.f - rect_h);
    Top->Rect()
        .SetPos(PD::fvec2(396, rect_pos))
        .SetSize(PD::fvec2(4, rect_h))
        .SetColor(gTheme.Slider);
  }

  if (PD::Hid::IsUp(PD::Hid::Key::Down) &&
      sp + cursor.pIndex < (int)pDL.size() - 1) {
    if (cursor.pIndex == 11) {
      sp++;
    } else {
      cursor++;
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Right) &&
      sp + cursor.pIndex + 5 < (int)pDL.size()) {
    if (cursor.pIndex == 11) {
      sp += 5;
    } else {
      if (cursor.pIndex + 5 > 11) {
        sp += (cursor.pIndex - 11 + 5);
        cursor.SetIndex(11);
      } else {
        cursor += 5;
      }
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Up) && cursor.pIndex + sp > 0) {
    if (cursor.pIndex == 0) {
      sp--;
    } else {
      cursor--;
    }
  }
  if (PD::Hid::IsUp(PD::Hid::Key::Left) && sp + cursor.pIndex - 5 >= 0) {
    if (cursor.pIndex == 0) {
      sp -= 5;
    } else {
      if (cursor.pIndex - 5 < 0) {
        sp -= 5 - cursor.pIndex;
        cursor.SetIndex(0);
      } else {
        cursor -= 5;
      }
    }
  }

  if (PD::Hid::IsDown(PD::Hid::Key::A)) {
    auto FSE = pDL.at(cursor.pIndex + sp);
    if (FSE->SubData.size() != 0) {
      pLastPos.push(std::make_pair(sp, cursor.GetIndex()));
      cursor.SetIndex(0);
      sp = 0;
      pStack.push(pDL);
      pDL = FSE->SubData;
    }
  }

  if (PD::Hid::IsDown(PD::Hid::Key::B)) {
    if (!pLastPos.empty()) {
      sp = pLastPos.top().first;
      cursor.SetIndex(pLastPos.top().second);
      pLastPos.pop();
    } else {
      sp = 0;
      cursor.SetIndex(0);
    }
    if (pStack.empty()) {
      Back();
    } else {
      pDL = pStack.top();
      pStack.pop();
    }
  }
}

/** MagicStr crates a string of the memory (unsafe) */
std::string MagicStr(PD::u32 v) {
  return std::format("0x{:08X} -> {}", v, std::string((char*)&v, 4));
}

/** Simply oneliner to fix stringstream problem with char values */
std::string _8Str(const PD::u8& v) { return std::format("0x{:02X}", v); }

/** Universal Type to hex string func */
template <typename T>
std::string _Str(const T& v) {
  if (sizeof(T) == 1) {
    return _8Str(v);
  }
  std::stringstream s;
  s << "0x" << std::uppercase << std::hex << std::setw(sizeof(T))
    << std::setfill('0') << v;
  return s.str();
}

/** Hex String + Value as integer */
template <typename T>
std::string _vStr(const T& v) {
  return std::format("{} -> {}", _Str(v), (PD::u32)v);
}

/** Type Value String like Endianness2String */
template <typename T>
std::string _fStr(const T& v, std::function<std::string(const T& v)> f) {
  return std::format("{} -> {}", _Str(v), f(v));
}

/**
 * Format Bytes value
 * does not work with every type but dont care cause its internals
 */
template <typename T>
std::string _bStr(const T& v) {
  return std::format("{} -> {}", _Str(v), PD::Strings::FormatBytes(v));
}

/** true false str */
template <typename T>
std::string _tStr(const T& v) {
  return std::format("{} -> {}", _Str(v), (bool)v);
}

/** Wrapper for Inspector::MakeEntry */
Inspector::TabEntry::Ref _me(const std::string& f, const std::string& e) {
  return Inspector::MakeEntry(f, e);
}

Inspector::TabEntry::Ref MakeRef(ctrff::BCSTM::Reference& b,
                                 const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("TypeID", _fStr<ctrff::BCSTM::ReferenceTypes>(
                                  (ctrff::BCSTM::ReferenceTypes)b.TypeID,
                                  ctrff::BCSTM::ReferenceType2String)));
  Lst.push_back(_me("Padding", _Str(b.Padding)));
  Lst.push_back(_me("Offset", _Str(b.Offset)));
  e->First = "Referene";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeSizedRef(ctrff::BCSTM::SizedReference& b,
                                      const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(MakeRef(b.Ref));
  Lst.push_back(_me("Size", _vStr(b.Size)));
  e->First = "SizedRef";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeRefListVec(std::vector<ctrff::BCSTM::Reference>& b,
                                        const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  for (size_t i = 0; i < b.size(); i++) {
    Lst.push_back(MakeRef(b[i], "Entry " + std::to_string(i)));
  }
  e->First = "Reference List";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeRefTable(ctrff::BCSTM::ReferenceTable& b,
                                      const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("Num References", _vStr(b.Count)));
  Lst.push_back(MakeRefListVec(b.Refs));
  e->First = "Reference Table";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeBlockHdr(ctrff::BCSTM::BlockHeader& b,
                                      const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("Magic", MagicStr(b.Magic)));
  Lst.push_back(_me("Size", _vStr(b.Size)));
  e->First = "Block Header";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeStreamInfo(ctrff::BCSTM::StreamInfo& b,
                                        const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("Encoding", _fStr<ctrff::BCSTM::Encoding>(
                                    (ctrff::BCSTM::Encoding)b.Encoding,
                                    ctrff::BCSTM::Encoding2String)));
  Lst.push_back(_me("Loop", _tStr(b.Loop)));
  Lst.push_back(_me("Channels", _vStr(b.ChannelCount)));
  Lst.push_back(_me("Padding", _Str(b.Padding)));
  Lst.push_back(_me("Sample Rate", _vStr(b.SampleRate)));
  Lst.push_back(_me("Loop Start", _vStr(b.LoopStartFrame)));
  Lst.push_back(_me("Loop End", _vStr(b.LoopEndFrame)));
  Lst.push_back(_me("Sample Blocks", _vStr(b.SampleBlockNum)));
  Lst.push_back(_me("Sample Block Size", _vStr(b.SampleBlockSize)));
  Lst.push_back(_me("Sample Block Samples", _vStr(b.SampleBlockSampleNum)));
  Lst.push_back(_me("Last Sample Block Size", _vStr(b.LastSampleBlockSize)));
  Lst.push_back(
      _me("Last Sample Block Samples", _vStr(b.LastSampleBlockSampleNum)));
  Lst.push_back(
      _me("Last Sample Block Padded Size", _vStr(b.LastSampleBlockPaddedSize)));
  Lst.push_back(_me("Seek Data Size", _vStr(b.SeekDataSize)));
  Lst.push_back(_me("Seek Interval Samples", _vStr(b.SeekIntervalSampleNum)));
  Lst.push_back(MakeRef(b.SampleDataRef, "Sample Data"));
  e->First = "Stream Info";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeInfoBlock(ctrff::BCSTM::InfoBlock& b,
                                       const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(MakeBlockHdr(b.Header));
  Lst.push_back(MakeRef(b.StreamInfoRef, "Stream Info"));
  Lst.push_back(MakeRef(b.TrackInfoTabRef, "Track Info Ref Table"));
  Lst.push_back(MakeRef(b.ChannelInfoTabRef, "Channel Info Ref Table"));
  Lst.push_back(MakeStreamInfo(b.StreamInfo));
  Lst.push_back(MakeRefTable(b.TrackInfoTab, "Track Info Table"));
  Lst.push_back(MakeRefTable(b.ChannelInfoTab, "Channel Info Table"));
  Lst.push_back(MakeRefListVec(b.ChannelInfoRefs, "Channel Info References"));
  e->First = "Info Block";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeDspAdpcmParam(ctrff::BCSTM::DSP_ADPCM_Param& b,
                                           const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  /** This code probably generates wrong information
   * but to lazy to look on that yet
   */
  for (int i = 0; i < 0x10; i += 4) {
    std::string res;
    for (int j = 0; j < 4; j++) {
      res += _Str(b.Coefficients[i + j]) + " ";
    }
    Lst.push_back(_me(std::to_string(i) + " - " + std::to_string(i + 3), res));
  }
  e->First = "DSP ADPCM Param";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeDspAdpcmContext(ctrff::BCSTM::DSP_ADPCM_Context& b,
                                             const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("Predictor / Scale", _Str(b.PredictorScale)));
  Lst.push_back(_me("Reserved", _Str(b.Reserved)));
  Lst.push_back(_me("Previous Sample", _vStr(b.PreviousSample)));
  Lst.push_back(_me("Second Previous Sample", _vStr(b.SecondPreviousSample)));
  e->First = "DSP ADPCM Context";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeDspAdpcmInfo(ctrff::BCSTM::DSP_ADPCM_Info& b,
                                          const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(MakeDspAdpcmParam(b.Param, "Param"));
  Lst.push_back(MakeDspAdpcmContext(b.Context, "Context"));
  Lst.push_back(MakeDspAdpcmContext(b.LoopContext, "Loop Context"));
  Lst.push_back(_me("Padding", _Str(b.Padding)));
  e->First = "DSP ADPCM Info";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeDspAdpcmInfoList(
    std::vector<ctrff::BCSTM::DSP_ADPCM_Info>& b,
    const std::string& name = "") {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  for (size_t i = 0; i < b.size(); i++) {
    Lst.push_back(MakeDspAdpcmInfo(b[i], "Entry " + std::to_string(i)));
  }
  e->First = "DSP ADPCM Info List";
  e->Second = name;
  e->SubData = Lst;
  return e;
}

Inspector::TabEntry::Ref MakeHeader(ctrff::BCSTM& b) {
  auto e = Inspector::TabEntry::New();
  std::vector<Inspector::TabEntry::Ref> Lst;
  Lst.push_back(_me("Magic", MagicStr(b.pHeader.Magic)));
  Lst.push_back(
      _me("Endianness", _fStr<ctrff::BCSTM::Endianness>(
                            (ctrff::BCSTM::Endianness)b.pHeader.Endianness,
                            ctrff::BCSTM::Endianness2String)));
  Lst.push_back(_me("HeaderSize", _vStr(b.pHeader.HeaderSize)));
  Lst.push_back(_me("Version", _vStr(b.pHeader.Version)));
  Lst.push_back(_me("FileSize", _bStr(b.pHeader.FileSize)));
  Lst.push_back(_me("NumBlocks", _vStr(b.pHeader.NumBlocks)));
  Lst.push_back(_me("Reserved", _Str(b.pHeader.Reserved)));
  Lst.push_back(MakeSizedRef(b.pInfoBlockRef, "Info Block"));
  Lst.push_back(MakeSizedRef(b.pSeekBlockRef, "Seek Block"));
  Lst.push_back(MakeSizedRef(b.pDataBlockRef, "Data Block"));
  e->First = "Header";
  e->SubData = Lst;
  return e;
}

void Inspector::ReadFile(const std::string& path) {
  List.clear();
  pPath = path;
  ctrff::BCSTM b;
  b.LoadFile(path);
  List.push_back(MakeHeader(b));
  List.push_back(MakeInfoBlock(b.pInfoBlock));
  List.push_back(MakeDspAdpcmInfoList(b.pDSP_ADPCM_Info));
  pDL = List;
}

Inspector::TabEntry::Ref Inspector::MakeEntry(const std::string& id,
                                              const std::string& val) {
  auto e = TabEntry::New();
  e->First = id;
  e->Second = val;
  return e;
}