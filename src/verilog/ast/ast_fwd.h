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

#ifndef CASCADE_SRC_VERILOG_AST_AST_FWD_H
#define CASCADE_SRC_VERILOG_AST_AST_FWD_H

namespace cascade {

class Node;
  class ArgAssign;
  class Attributes;
  class AttrSpec;
  class CaseGenerateItem;
  class CaseItem;
  class Event;
  class Expression;
    class BinaryExpression;
    class ConditionalExpression;
    class Primary;
      class Concatenation;
      class Identifier;
      class MultipleConcatenation;
      class Number;
      class String;
    class RangeExpression;
    class UnaryExpression;
  class GenerateBlock;
  class Id;
  class IfGenerateClause;
  class ModuleDeclaration;
  class ModuleItem;
    class Construct;
      class AlwaysConstruct;
      class GenerateConstruct;
        class ConditionalGenerateConstruct;
          class CaseGenerateConstruct;
          class IfGenerateConstruct;
        class LoopGenerateConstruct;
      class InitialConstruct;
    class ContinuousAssign;
    class Declaration;
      class GenvarDeclaration;
      class IntegerDeclaration;
      class LocalparamDeclaration;
      class NetDeclaration;
      class ParameterDeclaration;
      class RegDeclaration;
    class GenerateRegion;
    class Instantiation;
      class ModuleInstantiation;
    class PortDeclaration;
  class SimpleId;
  class Statement;
    class AssignStatement;
      class BlockingAssign;
      class NonblockingAssign;
    class BlockStatement;
      class ParBlock;
      class SeqBlock;
    class CaseStatement;
    class ConditionalStatement;
    class LoopStatement;
      class ForStatement;
      class ForeverStatement;
      class RepeatStatement;
      class WhileStatement;
    class TimingControlStatement;
    class SystemTaskEnableStatement;
      class DisplayStatement;
      class ErrorStatement;
      class FatalStatement;
      class FinishStatement;
      class InfoStatement;
      class RetargetStatement;
      class WarningStatement;
      class WriteStatement;
    class WaitStatement;
  class TimingControl;
    class DelayControl;
    class EventControl;
  class VariableAssign;

} // namespace cascade

#endif
