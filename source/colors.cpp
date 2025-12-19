#include <colors.hpp>

void Theme::Save(const std::string& path) {
  nlohmann::json js;
  js["info"]["version"] = pVer;
  js["colors"]["Header"] = Header.Hex();
  js["colors"]["Footer"] = Footer.Hex();
  js["colors"]["ListEven"] = ListEven.Hex();
  js["colors"]["ListOdd"] = ListOdd.Hex();
  js["colors"]["Selector"] = Selector.Hex();
  js["colors"]["Slider"] = Slider.Hex();
  js["colors"]["Text"] = Text.Hex();
  js["colors"]["Button"] = Button.Hex();
  js["colors"]["HblBG0"] = HblBG0.Hex();
  js["colors"]["Background"] = Background.Hex();
  js["colors"]["Progressbar"] = Progressbar.Hex();
  std::ofstream off(path, std::ios::out);
  off << js.dump(3);
  off.close();
}

void Theme::Load(const std::string& path) {
  nlohmann::json js;
  std::ifstream iff(path, std::ios::in);
  if (!iff) {
    Save(path);
    return;
  }
  iff >> js;
  iff.close();
  int v = 0;
  try {
    v = js["info"]["version"].get<int>();
  } catch (const nlohmann::json::exception& e) {
    Save(path);
    return;
  }

  if (v >= 0) {
    Header = PD::Color(js["colors"]["Header"].get<std::string>());
    Footer = PD::Color(js["colors"]["Footer"].get<std::string>());
    ListEven = PD::Color(js["colors"]["ListEven"].get<std::string>());
    ListOdd = PD::Color(js["colors"]["ListOdd"].get<std::string>());
    Selector = PD::Color(js["colors"]["Selector"].get<std::string>());
    Slider = PD::Color(js["colors"]["Slider"].get<std::string>());
    Text = PD::Color(js["colors"]["Text"].get<std::string>());
    Button = PD::Color(js["colors"]["Button"].get<std::string>());
    HblBG0 = PD::Color(js["colors"]["HblBG0"].get<std::string>());
    Background = PD::Color(js["colors"]["Background"].get<std::string>());
    Progressbar = PD::Color(js["colors"]["Progressbar"].get<std::string>());
  }

  Default(v);
  if (v != pVer) {
    Save(path);
  }
}