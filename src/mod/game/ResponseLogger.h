#pragma once

namespace lamina_sort::game {

/// Diagnostic only: logs every item stack request the server rejects, so a
/// sort step that the client predicted but the server refused (and vanilla
/// then rolled back) is visible in the log. It changes no behaviour.
class ResponseLogger {
public:
    static void install();
    static void uninstall();
};

} // namespace lamina_sort::game
