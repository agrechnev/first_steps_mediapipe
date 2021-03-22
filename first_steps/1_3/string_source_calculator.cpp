// From now on I put all calculators into separate cpp files
// Note that there is no h file!
// You don't want to include this class into main.cpp
// It is enough that it is registered with MP (some hidden global table ?)

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

//==============================================================================
namespace mediapipe{
    /// This is a source: a calculator that has no inputs
    class StringSourceCalculator : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // A single output "STR:" of type sdt::string
            cc->Outputs().Tag("STR").Set<string>();
            return OkStatus(); 
        }

        Status Process(CalculatorContext *cc) override {
            using namespace std;
            // This stream sends exactly 17 packets with timestamps 0 .. 16
            // Then sends the "STOP" status instead of "OK" to signal that the stream is finished
            // This works like CloseInputStream()
            if (t >= 17)
                return tool::StatusStop();
            
            // Construct a string and send
            string s = "JESSICA" + to_string(t);
            Packet p = MakePacket<string>(s).At(Timestamp(t));
            cc->Outputs().Tag("STR").AddPacket(p);
            // Increment the counter !
            t++;
            return OkStatus();
        }
    private:
        /// A counter, used to create ascending timestamps
        int t = 0;
    };

    REGISTER_CALCULATOR(StringSourceCalculator);
}
//==============================================================================