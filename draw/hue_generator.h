#include <random>
#include <chrono>


inline size_t GetNowAsSeconds()
{
    auto now = std::chrono::system_clock::now();

    return static_cast<size_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count());
}


class HueGenerator
{
public:
    HueGenerator(size_t seed)
        :
        generator_(seed),
        distribution_(0.0, 360.0)
    {

    }

    HueGenerator()
        :
        HueGenerator(GetNowAsSeconds())
    {

    }

    double MakeHue()
    {
        return this->distribution_(this->generator_);
    }

private:
    std::mt19937_64 generator_;
    std::uniform_real_distribution<double> distribution_;
};


