// Copyright 2018 The Contributors of rcgo
// All Contributions are owned by their respective authors.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "src/get_and_check_package_name.h"

// #include "error.h"
// #include "error_reporter.h"

namespace rcgo {

std::string GetAndCheckPackageName(const ast::SourceFiles& source_files,
                                   ErrorReporter* error_reporter) {
  struct PackageNameVisitor : public ast::DefaultNodeVisitor {
    ErrorReporter* error_reporter;

    explicit PackageNameVisitor(ErrorReporter* a_error_reporter)
        : error_reporter(a_error_reporter) {}

    // TODO(jrw972):  Cannot be the blank identifier.
    std::string package_name;

    void Visit(ast::SourceFile* ast) override {
      ast::Identifier* package = ast::Cast<ast::Identifier>(ast->package);
      if (package != NULL) {
        if (package_name.empty()) {
          package_name = package->identifier;
        } else if (package_name != package->identifier) {
          error_reporter->Insert(
              PackageMismatch(package->location, package_name,
                              package->identifier));
        }
      }
    }
  };

  PackageNameVisitor visitor(error_reporter);
  for (ast::SourceFiles::const_iterator pos = source_files.begin (),
           limit = source_files.end(); pos != limit; ++pos) {
    ast::Node* source_file = *pos;
    source_file->Accept(&visitor);
  }

  return visitor.package_name;
}

}  // namespace rcgo