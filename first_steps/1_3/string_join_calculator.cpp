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
    /// This calculator joins two strings
    class StringJoinCalculator : public CalculatorBase {
    public:
        static Status GetContract(CalculatorContract *cc) {
            using namespace std;
            // When calculator has more than one input or output, anonymous streams become messy
            // Here we demonstrate the use of tags, and also tag+number syntax
            // 2 inputs: "STR:0:" and "STR:1:", and one output "STR:" of type sdt::string
            cc->Inputs().Get("STR", 0).Set<string>();
            cc->Inputs().Get("STR", 1).Set<string>();
            cc->Outputs().Tag("STR").Set<string>();
            return OkStatus(); 
        }

       
        Status Process(CalculatorContext *cc) override {
            using namespace std;
            // Receive the two input packets
            Packet pIn1 = cc->Inputs().Get("STR", 0).Value();
            Packet pIn2 = cc->Inputs().Get("STR", 1).Value();
            // Extract strings, if the packet is not empty
            // Keep the default for an empty packet
            // MP automatically synchronizes streams by timestamps
            // If, say, timestamp 12 is missing in one stream, the packet will be empty
            // At least one input packet is always nonempty
            string s1("<EMPTY>"), s2("<EMPTY>");
            if (!pIn1.IsEmpty())
                s1 = pIn1.Get<string>();
            if (!pIn2.IsEmpty())
                s2 = pIn2.Get<string>();
            
            // Join 2 strings
            string s = s1 + s2;
            // Create the output packet and send
            Packet pOut = MakePacket<string>(s).At(cc->InputTimestamp());
            cc->Outputs().Tag("STR").AddPacket(pOut);
            return OkStatus();
        }
     };

     REGISTER_CALCULATOR(StringJoinCalculator);
}
//==============================================================================