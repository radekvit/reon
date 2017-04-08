#include <ctf.hpp>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
namespace reon_test {

class TestCase {
  string name_;

  Translation &translation_;
  string in_;

  string expected_;
  string out_;

 public:
  TestCase(const string &name, Translation &translation, const string &in,
           const string &expected)
      : name_(name), translation_(translation), in_(in) {
    std::ifstream expectedFile{expected};
    if (expectedFile.fail())
      throw std::invalid_argument("Could not open file with expected results.");

    std::stringstream buf;
    buf << expectedFile.rdbuf();
    expected_ = buf.str();
  }

  bool run() {
    std::stringstream out;
    std::ifstream in{in_};
    if (in.fail())
      throw std::invalid_argument("Could not open input file.");
    translation_.run(in, out);

    out_ = out.str();

    return out_ == expected_;
  }

  const string &name() const { return name_; }

  const string &result() const { return out_; }

  const string &expected() const { return expected_; }
};

}  // namespace reon_test