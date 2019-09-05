// Copyright (c) 2019 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>

#include "gmock/gmock.h"

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using AmdExtToKhrTest = PassTest<::testing::Test>;

using ::testing::HasSubstr;

std::string GetTest(std::string op_code, std::string new_op_code) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[undef:%\w+]] = OpUndef %uint
; CHECK-NEXT: )" + new_op_code +
                           R"( %uint %uint_3 Reduce [[undef]]
               OpCapability Shader
               OpCapability Groups
               OpExtension "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %uint
          %8 = )" + op_code +
                           R"( %uint %uint_3 Reduce %7
               OpReturn
               OpFunctionEnd

)";
  return text;
}

TEST_F(AmdExtToKhrTest, ReplaceGroupIAddNonUniformAMD) {
  std::string text =
      GetTest("OpGroupIAddNonUniformAMD", "OpGroupNonUniformIAdd");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupFAddNonUniformAMD) {
  std::string text =
      GetTest("OpGroupFAddNonUniformAMD", "OpGroupNonUniformFAdd");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupUMinNonUniformAMD) {
  std::string text =
      GetTest("OpGroupUMinNonUniformAMD", "OpGroupNonUniformUMin");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupSMinNonUniformAMD) {
  std::string text =
      GetTest("OpGroupSMinNonUniformAMD", "OpGroupNonUniformSMin");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupFMinNonUniformAMD) {
  std::string text =
      GetTest("OpGroupFMinNonUniformAMD", "OpGroupNonUniformFMin");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupUMaxNonUniformAMD) {
  std::string text =
      GetTest("OpGroupUMaxNonUniformAMD", "OpGroupNonUniformUMax");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupSMaxNonUniformAMD) {
  std::string text =
      GetTest("OpGroupSMaxNonUniformAMD", "OpGroupNonUniformSMax");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceGroupFMaxNonUniformAMD) {
  std::string text =
      GetTest("OpGroupFMaxNonUniformAMD", "OpGroupNonUniformFMax");
  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceMbcntAMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_ballot"
; CHECK: OpDecorate [[var:%\w+]] BuiltIn SubgroupLtMask
; CHECK: [[var]] = OpVariable %_ptr_Input_v4uint Input
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[ld:%\w+]] = OpLoad %v4uint [[var]]
; CHECK-NEXT: [[shuffle:%\w+]] = OpVectorShuffle %v2uint [[ld]] [[ld]] 0 1
; CHECK-NEXT: [[bitcast:%\w+]] = OpBitcast %ulong [[shuffle]]
; CHECK-NEXT: [[and:%\w+]] = OpBitwiseAnd %ulong [[bitcast]] %ulong_0
; CHECK-NEXT: [[result:%\w+]] = OpBitCount %uint [[and]]
               OpCapability Shader
               OpCapability Int64
               OpExtension "SPV_AMD_shader_ballot"
          %1 = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "func"
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
    %ulong_0 = OpConstant %ulong 0
          %2 = OpFunction %void None %4
          %8 = OpLabel
          %9 = OpExtInst %uint %1 MbcntAMD %ulong_0
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceSwizzleInvocationsAMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_ballot"
; CHECK: OpDecorate [[var:%\w+]] BuiltIn SubgroupLocalInvocationId
; CHECK: [[subgroup:%\w+]] = OpConstant %uint 3
; CHECK: [[offset:%\w+]] = OpConstantComposite %v4uint
; CHECK: [[var]] = OpVariable %_ptr_Input_uint Input
; CHECK: [[uint_max:%\w+]] = OpConstant %uint 4294967295
; CHECK: [[ballot_value:%\w+]] = OpConstantComposite %v4uint [[uint_max]] [[uint_max]] [[uint_max]] [[uint_max]]
; CHECK: [[null:%\w+]] = OpConstantNull [[type:%\w+]]
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[data:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[id:%\w+]] = OpLoad %uint [[var]]
; CHECK-NEXT: [[quad_idx:%\w+]] = OpBitwiseAnd %uint [[id]] %uint_3
; CHECK-NEXT: [[quad_ldr:%\w+]] = OpBitwiseXor %uint [[id]] [[quad_idx]]
; CHECK-NEXT: [[my_offset:%\w+]] = OpVectorExtractDynamic %uint [[offset]] [[quad_idx]]
; CHECK-NEXT: [[target_inv:%\w+]] = OpIAdd %uint [[quad_ldr]] [[my_offset]]
; CHECK-NEXT: [[is_active:%\w+]] = OpGroupNonUniformBallotBitExtract %bool [[subgroup]] [[ballot_value]] [[target_inv]]
; CHECK-NEXT: [[shuffle:%\w+]] = OpGroupNonUniformShuffle [[type]] [[subgroup]] [[data]] [[target_inv]]
; CHECK-NEXT: [[result:%\w+]] = OpSelect [[type]] [[is_active]] [[shuffle]] [[null]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_ballot"
        %ext = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_x = OpConstant %uint 1
     %uint_y = OpConstant %uint 2
     %uint_z = OpConstant %uint 3
     %uint_w = OpConstant %uint 0
     %v4uint = OpTypeVector %uint 4
     %offset = OpConstantComposite %v4uint %uint_x %uint_y %uint_z %uint_x
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %data = OpUndef %uint
          %9 = OpExtInst %uint %ext SwizzleInvocationsAMD %data %offset
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}
TEST_F(AmdExtToKhrTest, ReplaceSwizzleInvocationsMaskedAMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_ballot"
; CHECK: OpDecorate [[var:%\w+]] BuiltIn SubgroupLocalInvocationId
; CHECK: [[x:%\w+]] = OpConstant %uint 19
; CHECK: [[y:%\w+]] = OpConstant %uint 12
; CHECK: [[z:%\w+]] = OpConstant %uint 16
; CHECK: [[var]] = OpVariable %_ptr_Input_uint Input
; CHECK: [[mask_extend:%\w+]] = OpConstant %uint 4294967264
; CHECK: [[uint_max:%\w+]] = OpConstant %uint 4294967295
; CHECK: [[subgroup:%\w+]] = OpConstant %uint 3
; CHECK: [[ballot_value:%\w+]] = OpConstantComposite %v4uint [[uint_max]] [[uint_max]] [[uint_max]] [[uint_max]]
; CHECK: [[null:%\w+]] = OpConstantNull [[type:%\w+]]
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[data:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[id:%\w+]] = OpLoad %uint [[var]]
; CHECK-NEXT: [[and_mask:%\w+]] = OpBitwiseOr %uint [[x]] [[mask_extend]]
; CHECK-NEXT: [[and:%\w+]] = OpBitwiseAnd %uint [[id]] [[and_mask]]
; CHECK-NEXT: [[or:%\w+]] = OpBitwiseOr %uint [[and]] [[y]]
; CHECK-NEXT: [[target_inv:%\w+]] = OpBitwiseXor %uint [[or]] [[z]]
; CHECK-NEXT: [[is_active:%\w+]] = OpGroupNonUniformBallotBitExtract %bool [[subgroup]] [[ballot_value]] [[target_inv]]
; CHECK-NEXT: [[shuffle:%\w+]] = OpGroupNonUniformShuffle [[type]] [[subgroup]] [[data]] [[target_inv]]
; CHECK-NEXT: [[result:%\w+]] = OpSelect [[type]] [[is_active]] [[shuffle]] [[null]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_ballot"
        %ext = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_x = OpConstant %uint 19
     %uint_y = OpConstant %uint 12
     %uint_z = OpConstant %uint 16
     %v3uint = OpTypeVector %uint 3
       %mask = OpConstantComposite %v3uint %uint_x %uint_y %uint_z
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %data = OpUndef %uint
          %9 = OpExtInst %uint %ext SwizzleInvocationsMaskedAMD %data %mask
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceWriteInvocationAMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_ballot"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_ballot"
; CHECK: OpDecorate [[var:%\w+]] BuiltIn SubgroupLocalInvocationId
; CHECK: [[var]] = OpVariable %_ptr_Input_uint Input
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[input_val:%\w+]] = OpUndef %uint
; CHECK-NEXT: [[write_val:%\w+]] = OpUndef %uint
; CHECK-NEXT: [[ld:%\w+]] = OpLoad %uint [[var]]
; CHECK-NEXT: [[cmp:%\w+]] = OpIEqual %bool [[ld]] %uint_3
; CHECK-NEXT: [[result:%\w+]] = OpSelect %uint [[cmp]] [[write_val]] [[input_val]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_ballot"
        %ext = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %uint
          %8 = OpUndef %uint
          %9 = OpExtInst %uint %ext WriteInvocationAMD %7 %8 %uint_3
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceFMin3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeFloat 32
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] FMin [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] FMin [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %float
          %8 = OpUndef %float
          %9 = OpUndef %float
         %10 = OpExtInst %float %ext FMin3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceSMin3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 1
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] SMin [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] SMin [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %int
          %8 = OpUndef %int
          %9 = OpUndef %int
         %10 = OpExtInst %int %ext SMin3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceUMin3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 0
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] UMin [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] UMin [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %uint
          %8 = OpUndef %uint
          %9 = OpUndef %uint
         %10 = OpExtInst %uint %ext UMin3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceFMax3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeFloat 32
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] FMax [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] FMax [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %float
          %8 = OpUndef %float
          %9 = OpUndef %float
         %10 = OpExtInst %float %ext FMax3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceSMax3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 1
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] SMax [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] SMax [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %int
          %8 = OpUndef %int
          %9 = OpUndef %int
         %10 = OpExtInst %int %ext SMax3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceUMax3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 0
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %uint
          %8 = OpUndef %uint
          %9 = OpUndef %uint
         %10 = OpExtInst %uint %ext UMax3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceVecUMax3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeVector
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[temp:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[x]] [[y]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[temp]] [[z]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
        %vec = OpTypeVector %uint 4
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %vec
          %8 = OpUndef %vec
          %9 = OpUndef %vec
         %10 = OpExtInst %vec %ext UMax3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceFMid3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeFloat 32
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[min:%\w+]] = OpExtInst [[type]] [[ext]] FMin [[y]] [[z]]
; CHECK-NEXT: [[max:%\w+]] = OpExtInst [[type]] [[ext]] FMax [[y]] [[z]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] FClamp [[x]] [[min]] [[max]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %float
          %8 = OpUndef %float
          %9 = OpUndef %float
         %10 = OpExtInst %float %ext FMid3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceSMid3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 1
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[min:%\w+]] = OpExtInst [[type]] [[ext]] SMin [[y]] [[z]]
; CHECK-NEXT: [[max:%\w+]] = OpExtInst [[type]] [[ext]] SMax [[y]] [[z]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] SClamp [[x]] [[min]] [[max]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %int
          %8 = OpUndef %int
          %9 = OpUndef %int
         %10 = OpExtInst %int %ext SMid3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceUMid3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeInt 32 0
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[min:%\w+]] = OpExtInst [[type]] [[ext]] UMin [[y]] [[z]]
; CHECK-NEXT: [[max:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[y]] [[z]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] UClamp [[x]] [[min]] [[max]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %uint
          %8 = OpUndef %uint
          %9 = OpUndef %uint
         %10 = OpExtInst %uint %ext UMid3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, ReplaceVecUMid3AMD) {
  const std::string text = R"(
; CHECK: OpCapability Shader
; CHECK-NOT: OpExtension "SPV_AMD_shader_trinary_minmax"
; CHECK-NOT: OpExtInstImport "SPV_AMD_shader_trinary_minmax"
; CHECK: [[ext:%\w+]] = OpExtInstImport "GLSL.std.450"
; CHECK: [[type:%\w+]] = OpTypeVector
; CHECK: OpFunction
; CHECK-NEXT: OpLabel
; CHECK-NEXT: [[x:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[y:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[z:%\w+]] = OpUndef [[type]]
; CHECK-NEXT: [[min:%\w+]] = OpExtInst [[type]] [[ext]] UMin [[y]] [[z]]
; CHECK-NEXT: [[max:%\w+]] = OpExtInst [[type]] [[ext]] UMax [[y]] [[z]]
; CHECK-NEXT: [[result:%\w+]] = OpExtInst [[type]] [[ext]] UClamp [[x]] [[min]] [[max]]
               OpCapability Shader
               OpExtension "SPV_AMD_shader_trinary_minmax"
        %ext = OpExtInstImport "SPV_AMD_shader_trinary_minmax"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "func"
               OpExecutionMode %1 OriginUpperLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %vec = OpTypeVector %uint 3
       %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
     %uint_3 = OpConstant %uint 3
          %1 = OpFunction %void None %3
          %6 = OpLabel
          %7 = OpUndef %vec
          %8 = OpUndef %vec
          %9 = OpUndef %vec
         %10 = OpExtInst %vec %ext UMid3AMD %7 %8 %9
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<AmdExtensionToKhrPass>(text, true);
}

TEST_F(AmdExtToKhrTest, SetVersion) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability Int64
               OpExtension "SPV_AMD_shader_ballot"
          %1 = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "func"
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
    %ulong_0 = OpConstant %ulong 0
          %2 = OpFunction %void None %4
          %8 = OpLabel
          %9 = OpExtInst %uint %1 MbcntAMD %ulong_0
               OpReturn
               OpFunctionEnd
)";

  // Set the version to 1.1 and make sure it is upgraded to 1.3.
  SetTargetEnv(SPV_ENV_UNIVERSAL_1_1);
  SetDisassembleOptions(0);
  auto result = SinglePassRunAndDisassemble<AmdExtensionToKhrPass>(
      text, /* skip_nop = */ true, /* skip_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
  const std::string& output = std::get<0>(result);
  EXPECT_THAT(output, HasSubstr("Version: 1.3"));
}

TEST_F(AmdExtToKhrTest, SetVersion1) {
  const std::string text = R"(
               OpCapability Shader
               OpCapability Int64
               OpExtension "SPV_AMD_shader_ballot"
          %1 = OpExtInstImport "SPV_AMD_shader_ballot"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "func"
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
      %ulong = OpTypeInt 64 0
    %ulong_0 = OpConstant %ulong 0
          %2 = OpFunction %void None %4
          %8 = OpLabel
          %9 = OpExtInst %uint %1 MbcntAMD %ulong_0
               OpReturn
               OpFunctionEnd
)";

  // Set the version to 1.4 and make sure it is stays the same.
  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetDisassembleOptions(0);
  auto result = SinglePassRunAndDisassemble<AmdExtensionToKhrPass>(
      text, /* skip_nop = */ true, /* skip_validation = */ false);

  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
  const std::string& output = std::get<0>(result);
  EXPECT_THAT(output, HasSubstr("Version: 1.4"));
}

}  // namespace
}  // namespace opt
}  // namespace spvtools