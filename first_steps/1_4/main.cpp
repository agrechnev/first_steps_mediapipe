/// Example 1.4 : Calculator options
/// By Oleksiy Grechnyev, IT-JIM
/// Here I create a custom calculator with options
/// It is ideologically simple, but technically tricky
/// We define a data class GoblinCalculator14Options which is a protobuff class
/// Defined in goblin_calculator14.proto
/// Then options are supplied in the graph
/// Note the funny monkey business in BUILD !

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

//==============================================================================
mediapipe::Status run(){
    using namespace std;
    // Create the graph using our custom calculator GoblinCalculator14
    // Note how you supply options to a calculator
    // Standard MP calculators use options a lot, get used to this somewhat bizarre syntax!
    // Try to modify the two options!
    string protoG = R"(
    input_stream: "in"
    output_stream: "out"
    node {
        calculator: "GoblinCalculator14"
        input_stream: "in"
        output_stream: "out"
        options : {
            [mediapipe.GoblinCalculator14Options.ext]{
                opt_a: 7.0
                opt_b: 3.0
            }   
        }
    }
    )";

    // Let us now parse the string properly this time, checking if the parse is successful
    mediapipe::CalculatorGraphConfig config;
    if (!mediapipe::ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        // mediapipe::Status is actually absl::Status (at least in the current mediapipe)
        // So we can create BAD statuses like this
        return absl::InternalError("Cannot parse the graph config !");
    } 

    // Create MP Graph and intialize it with config
    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    // Add observer to "out:
    auto cb = [](const mediapipe::Packet &packet)->mediapipe::Status{
        cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<double>() << endl;
        return mediapipe::OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));

    /// Run the graph
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    // Send input packets to the graph, stream "in", then close it
    for (int i=0; i<13; ++i) {
        mediapipe::Timestamp ts(i);
        mediapipe::Packet packet = mediapipe::MakePacket<double>(i*0.1).At(ts);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
    }
    graph.CloseInputStream("in");

    // Wait for the graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return mediapipe::OkStatus();
}

//==============================================================================
int main(){
    using namespace std;
    cout << "Example 1.4 : Calculator options" << endl;
    // Call run(), which return a status
    mediapipe::Status status = run();
    cout << "status = " << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}