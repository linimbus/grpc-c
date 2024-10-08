// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

// Copyright (c) 2008-2013, Dave Benson.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Modified to implement C code by Dave Benson.

#include <algorithm>
#include <map>

#include "grpc_c_helpers.h"
#include "grpc_c_message.h"
#include <protoc-c/c_helpers.h>

#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace grpc_c {

// ===================================================================

GrpcCMessageGenerator::GrpcCMessageGenerator(const Descriptor *descriptor,
                                             const std::string &dllexport_decl)
    : descriptor_(descriptor), dllexport_decl_(dllexport_decl),
      grpc_c_nested_generators_(
          new std::unique_ptr<
              GrpcCMessageGenerator>[descriptor->nested_type_count()]) {

  for (int i = 0; i < descriptor->nested_type_count(); i++) {
    grpc_c_nested_generators_[i].reset(
        new GrpcCMessageGenerator(descriptor->nested_type(i), dllexport_decl));
  }
}

GrpcCMessageGenerator::~GrpcCMessageGenerator() {}

void GrpcCMessageGenerator::GenerateStructTypedef(io::Printer *printer) {

  printer->Print("typedef struct _$classname$ $grpc_c_classname$;\n",
                 "classname",
                 c::FullNameToC(descriptor_->full_name(), descriptor_->file()),
                 "grpc_c_classname", FullNameToGrpcC(descriptor_->full_name()));

  printer->Print(
      "#define $grpc_c_classname$__init $lcfullname$__init\n",
      "grpc_c_classname", FullNameToGrpcC(descriptor_->full_name()),
      "lcfullname",
      c::FullNameToLower(descriptor_->full_name(), descriptor_->file()));

  printer->Print(
      "#define $grpc_c_classname$__free_unpacked $lcfullname$__free_unpacked\n",
      "grpc_c_classname", FullNameToGrpcC(descriptor_->full_name()),
      "lcfullname",
      c::FullNameToLower(descriptor_->full_name(), descriptor_->file()));

  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    grpc_c_nested_generators_[i]->GenerateStructTypedef(printer);
  }
}

MessagePackUnpackGenerator::MessagePackUnpackGenerator(
    const Descriptor *descriptor, const std::string &dllexport_decl)
    : descriptor_(descriptor), dllexport_decl_(dllexport_decl) {}

MessagePackUnpackGenerator::~MessagePackUnpackGenerator() {}

void MessagePackUnpackGenerator::GenerateHelperFunctionDeclarations(
    io::Printer *printer) {
  std::map<std::string, std::string> vars;
  vars["classname"] =
      c::FullNameToC(descriptor_->full_name(), descriptor_->file());
  vars["lcclassname"] =
      c::FullNameToLower(descriptor_->full_name(), descriptor_->file());
  printer->Print(vars, "\n/* $lcclassname$ packer and unpacker methods */\n"
                       "size_t $lcclassname$_packer (void *input, "
                       "grpc_byte_buffer **buffer);\n");

  printer->Print(vars, "void *$lcclassname$_unpacker (grpc_c_context_t "
                       "*context, grpc_byte_buffer *buffer);");

  printer->Print(vars, "\nvoid $lcclassname$_free ($classname$ * buf);\n");
}

static int compare_pfields_by_number(const void *a, const void *b) {
  const FieldDescriptor *pa = *(const FieldDescriptor **)a;
  const FieldDescriptor *pb = *(const FieldDescriptor **)b;
  if (pa->number() < pb->number())
    return -1;
  if (pa->number() > pb->number())
    return +1;
  return 0;
}

void MessagePackUnpackGenerator::GenerateHelperFunctionDefinitions(
    io::Printer *printer) {
  std::map<std::string, std::string> vars;
  vars["classname"] =
      c::FullNameToC(descriptor_->full_name(), descriptor_->file());
  vars["lcclassname"] =
      c::FullNameToLower(descriptor_->full_name(), descriptor_->file());

  printer->Print(
      vars, "\nsize_t\n"
            "$lcclassname$_packer (void *input, grpc_byte_buffer **buffer)\n"
            "{\n"
            "    uint8_t *out;\n"
            "    size_t size = $lcclassname$__get_packed_size"
            "(($classname$ *)input);\n"
            "    out = gpr_malloc(sizeof(uint8_t) * size);\n"
            "    size_t len = $lcclassname$__pack(($classname$ *)input, "
            "out);\n"
            "    grpc_slice slice = grpc_slice_new(out, len, gpr_free);\n"
            "    *buffer = grpc_raw_byte_buffer_create(&slice, 1);\n"
            "    grpc_slice_unref(slice);\n"
            "    return len;\n"
            "}\n");

  printer->Print(
      vars, "\nvoid *\n"
            "$lcclassname$_unpacker (grpc_c_context_t *context, "
            "grpc_byte_buffer *buffer)\n"
            "{\n"
            "    $classname$ *h = NULL;\n"
            "    if (buffer != NULL) {\n"
            "        struct ProtobufCAllocator allocator;\n"
            "        grpc_byte_buffer_reader reader;\n"
            "        grpc_byte_buffer_reader_init(&reader, buffer);\n"
            "        grpc_slice slice;\n"
            "        char *buf = NULL;\n"
            "\n"
            "        slice = grpc_byte_buffer_reader_readall(&reader);\n"
            "        buf = gpr_malloc(GRPC_SLICE_LENGTH(slice));\n"
            "        memcpy(buf, GRPC_SLICE_START_PTR(slice), "
            "GRPC_SLICE_LENGTH(slice));\n"
            "\n"
            "        h = "
            "$lcclassname$__unpack(grpc_c_get_protobuf_c_allocator(context, "
            "&allocator), GRPC_SLICE_LENGTH(slice), (void *)buf);\n"
            "        gpr_free(buf);\n"
            "        grpc_slice_unref(slice);\n"
            "    }\n"
            "    return h;\n"
            "}\n");

  printer->Print(vars, "\nvoid $lcclassname$_free ($classname$ * buf)\n"
                       "{\n"
                       "    struct ProtobufCAllocator allocator;\n"
                       "    if (buf == NULL) return;\n"
                       "    $lcclassname$__free_unpacked(buf, "
                       "grpc_c_get_protobuf_c_allocator(NULL, &allocator));\n"
                       "}\n");
}

} // namespace grpc_c
} // namespace compiler
} // namespace protobuf
} // namespace google
