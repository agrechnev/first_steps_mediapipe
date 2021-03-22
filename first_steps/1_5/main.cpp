/// Example 1.5 : Calculator with side packets
/// By Oleksiy Grechnyev, IT-JIM
/// Here I create a custom calculator with side packets

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

//==============================================================================
mediapipe::Status run(){
    using namespace std;
    // From now on I'll use mediapipe namespace
    using namespace mediapipe;
    // Create the graph using our custom calculator GoblinCalculator15
    // Here the graph has 2 input side packets a and b
    string protoG = R"(
    input_stream: "in"
    output_stream: "out"
    input_side_packet : "a"
    input_side_packet : "b"
    node {
        calculator: "GoblinCalculator15"
        input_side_packet : "a"
        input_stream: "in"
        output_stream: "out"
    }
    )";

    // Let us now parse the string properly this time, checking if the parse is successful
    CalculatorGraphConfig config;
    if (!ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        // mediapipe::Status is actually absl::Status (at least in the current mediapipe)
        // So we can create BAD statuses like this
        return absl::InternalError("Cannot parse the graph config !");
    } 

    // Create MP Graph and intialize it with config
    CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    // Add observer to "out:
    auto cb = [](const Packet &packet)->Status{
        cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<double>() << endl;
        return OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));

    Packet sideA = MakePacket<double>(7.0);
    /// Run the graph, supplying side packets at this point!
    /// The expression in curly braces is an std::map literal !
    MP_RETURN_IF_ERROR(graph.StartRun({{"a", sideA}}));

    // Send input packets to the graph, stream "in", then close it
    for (int i=0; i<13; ++i) {
        Timestamp ts(i);
        Packet packet = MakePacket<double>(i*0.1).At(ts);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
    }
    graph.CloseInputStream("in");

    // Wait for the graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return OkStatus();
}

//==============================================================================
int main(){
    using namespace std;
    cout << "Example 1.5 : Calculator with side packets" << endl;
    // Call run(), which return a status
    mediapipe::Status status = run();
    cout << "status = " << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}