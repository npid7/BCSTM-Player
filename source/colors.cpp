#include <colors.hpp>

void Theme::Save(const std::string& path) {
  nlohmann::json js;
  js["Header"] = Header.Hex(true);
  js["Footer"] = Footer.Hex(true);
  js["ListEven"] = ListEven.Hex(true);
  js["ListOdd"] = ListOdd.Hex(true);
  js["Selector"] = Selector.Hex(true);
  js["Slider"] = Slider.Hex(true);
  js["Text"] = Text.Hex(true);
  js["Button"] = Button.Hex(true);
  js["HblBG0"] = HblBG0.Hex(true);
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
  Header = PD::Color(js["Header"].get<std::string>());
  Footer = PD::Color(js["Footer"].get<std::string>());
  ListEven = PD::Color(js["ListEven"].get<std::string>());
  ListOdd = PD::Color(js["ListOdd"].get<std::string>());
  Selector = PD::Color(js["Selector"].get<std::string>());
  Slider = PD::Color(js["Slider"].get<std::string>());
  Text = PD::Color(js["Text"].get<std::string>());
  Button = PD::Color(js["Button"].get<std::string>());
  HblBG0 = PD::Color(js["HblBG0"].get<std::string>());
}