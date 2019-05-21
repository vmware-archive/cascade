// Copyright 2017-2019 VMware, Inc.
// SPDX-License-Identifier: BSD-2-Clause
//
// The BSD-2 license (the License) set forth below applies to all parts of the
// Cascade project.  You may not use this file except in compliance with the
// License.
//
// BSD-2 License
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>
#include "base/system/system.h"
#include "benchmark/benchmark.h"
#include "cascade/cascade.h"
#include "cl/cl.h"
#include "harness.h"
#include "ui/view.h"
#include "verilog/parse/parser.h"

using namespace cascade;
using namespace cascade::cl;
using namespace std;

BENCHMARK_MAIN();

static void BM_Parser(benchmark::State& state, string code) {
    stringstream ss(code);

    Log log;
    Parser p(&log);

    for(auto _ : state) {
        ss.clear();
        ss.str(code);
        p.set_stream(ss);
        p.parse();
    }
}

BENCHMARK_CAPTURE(BM_Parser, parse_empty, "");

BENCHMARK_CAPTURE(BM_Parser, parse_and, 
R"verilog(
    module and(x,y,z);
        input wire x;
        input wire y;
        output wire z;

        assign z = x & y;
    endmodule
)verilog");

BENCHMARK_CAPTURE(BM_Parser, parse_array, 
R"verilog(
    module Array();

    parameter W = 2;
    parameter FINISH = 0;

    localparam N = (1 << W) - 1;
    localparam K = 3*W;

    reg[W-1:0] low;
    reg[K:0]   mid  [N:0];    
    reg        high [1:0][1:0][1:0];

    reg[63:0] COUNT = 0;
    always @(posedge clock.val) begin
        low <= low + 1;
        mid[low] <= mid[low] + 1; 
        high[mid[N][K]][mid[N][1]][mid[N][0]] <= high[mid[N][K]][mid[N][1]][mid[N][0]] + 1;

        if (high[1][0][0]) begin
        $display(COUNT);
        $finish(FINISH); 
        end else
        COUNT <= COUNT + 1;
    end

    endmodule

    Array#(.W(7)) array();
)verilog");

static void BM_Code(benchmark::State& state, string code) {
    for(auto _ : state) {
        stringstream ss;

        Cascade c;
        c.set_include_path(System::src_root());
        c.attach_view(new PView(ss));
        c.run();

        c.eval() << "`include \"data/march/minimal.v\"\n" << code << endl;

        c.wait_for_stop();
    }
}

BENCHMARK_CAPTURE(BM_Code, code_empty, R"verilog(initial $finish;)verilog");

static void BM_CodeArray(benchmark::State& state) {
    const string array_code = R"verilog(
    module Array();

    parameter W = 2;
    parameter FINISH = 0;

    localparam N = (1 << W) - 1;
    localparam K = 3*W;

    reg[W-1:0] low;
    reg[K:0]   mid  [N:0];    
    reg        high [1:0][1:0][1:0];

    reg[63:0] COUNT = 0;
    always @(posedge clock.val) begin
        low <= low + 1;
        mid[low] <= mid[low] + 1; 
        high[mid[N][K]][mid[N][1]][mid[N][0]] <= high[mid[N][K]][mid[N][1]][mid[N][0]] + 1;

        if (high[1][0][0]) begin
        $display(COUNT);
        $finish(FINISH); 
        end else
        COUNT <= COUNT + 1;
    end

    endmodule
    )verilog";

    const string instantiation_code = "Array#(.W(" + to_string(state.range(0)) + ")) array();";

    for(auto _ : state) {
        stringstream ss;

        Cascade c;
        c.set_include_path(System::src_root());
        c.attach_view(new PView(ss));
        c.run();

        c.eval() << "`include \"data/march/minimal.v\"\n" << array_code << instantiation_code << endl;

        c.wait_for_stop();
    }
}

BENCHMARK(BM_CodeArray)->Range(2,5)->Complexity();
