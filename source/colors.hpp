#pragma once

#include <palladium>

class Theme {
 public:
  Theme() { Default(); }
  ~Theme() {}

  void Save(const std::string& path);
  void Load(const std::string& path);

  void Default(int ver = -1) {
    if (ver < 0) {
      Header = PD::Color("#111111");
      Footer = Header;
      ListEven = PD::Color("#222222aa");
      ListOdd = PD::Color("#333333aa");
      Selector = PD::Color("#4444aacc");
      Slider = PD::Color("#444444ff");
      Text = PD::Color("#ffffff");
      Button = PD::Color("#4444aacc");
      HblBG0 = PD::Color("#fdc964");
      Background = PD::Color("#333333ff");
      Progressbar = PD::Color("#4444aa");
    }
  }

  PD::Color Header;
  PD::Color Footer;
  PD::Color ListEven;
  PD::Color ListOdd;
  PD::Color Selector;
  PD::Color Slider;
  PD::Color Text;
  PD::Color Button;
  PD::Color HblBG0;
  PD::Color Background;
  PD::Color Progressbar;

  const int pVer = 0;
};