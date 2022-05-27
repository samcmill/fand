@CODE_COVERAGE_RULES@
CODE_COVERAGE_IGNORE_PATTERN = "$(abs_top_srcdir)/src/3rdparty/*"
CODE_COVERAGE_LCOV_SHOPTS = --no-external

SUBDIRS = configs src

dist_doc_DATA = README.md
EXTRA_DIST = .clang-format INSTALL LICENSE

if HAVE_CLANG_FORMAT
.PHONY: check-format
check-format:
	@for subdir in $(SUBDIRS); do \
		find $$subdir \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "*3rdparty*" -exec sh -c '$(CLANG_FORMAT) -style=file {} | diff -q {} -' \; ; \
	done

.PHONY: format
format:
	@for subdir in $(SUBDIRS); do \
		find $$subdir \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "*3rdparty*" -exec $(CLANG_FORMAT) -i -style=file {} + ; \
	done
else
.PHONY: check-format format
check-format format:
	@echo "clang-format not found"
endif
