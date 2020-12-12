#ifndef RESTRICTED_FRAME_PARSER_H
#define RESTRICTED_FRAME_PARSER_H

#include "third_party/blink/renderer/core/script/html_parser_script_runner.h"
#include "third_party/blink/renderer/core/html/parser/html_document_parser.h"

namespace blink {

class CORE_EXPORT RestrictedFrameParser {
 public:
  static bool PostRestrictedTaskIfNecessary(
          HTMLParserScriptRunner* script_runner,
          Element* script_element,
          const TextPosition& script_start_position);

  static void PostNormalTaskToContinueParsing(
          base::OnceClosure callback);

};
}

#endif // RESTRICTED_FRAME_PARSER_H
