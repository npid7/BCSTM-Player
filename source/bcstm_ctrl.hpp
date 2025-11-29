#pragma once

#include <bcstm/bcstmv2.hpp>
#include <bcstm/ctrff_decode.hpp>

struct BCSTM_Ctrl {
  enum ReqType {
    OpenFile,   /** Big Overhead so should be handled in a separate thread */
    CloseFile,  /** No Overhead */
    Play,       /** No visible overhead */
    Pause,      /** Can be direct accessded due to no overhead */
    Stop,       /** No Overhead */
    KillThread, /** Should be called on close */
    SwitchDec,  /** Stops and switches decoder */
  };
  struct Request {
    Request(ReqType t, const std::string& d) {
      req = t;
      req_dat = d;
    }
    ReqType req;
    std::string req_dat;
  };

  void DoRequest(ReqType t, const std::string& dat = "") {
    pRequests.PushBack(Request(t, dat));
  }

  D7::BCSTMPlayerBase* player = nullptr;
  PD::List<Request> pRequests;
  bool pFileLoaded = false;
};

extern BCSTM_Ctrl bcstm_ctrl;