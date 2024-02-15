#include "magic_builder.hpp"

#include <concepts>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

class Commit {

};

struct CommitData {
    std::string message;
    std::vector<std::filesystem::path> included_paths;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> timestamp;
    std::vector<std::string> authors;
    std::optional<std::string> fixup_hash;
    std::optional<std::string> squash_hash;
};

struct CommitVerifier : magic_bldr::Verifier<CommitVerifier> {
    enum class Action {
        AddMessage,
        AddFolder,
        AddFile,
        AddAll,
        SetTimestamp,
        SetTimestampNow,
        SetAuthorSelf,
        SetAuthor,
        SetMultipleAuthors,
        SetFixup,
        SetSquash,
    };

    bool has_message = false;
    bool has_adds = false;
    bool has_added_all = false;
    bool has_timestamp = false;
    bool has_author = false;
    bool has_fixup_or_squash = false;

    consteval bool ready(CommitVerifier d) {
        return has_message && has_adds && has_timestamp && has_author;
    };

    consteval CommitVerifier state_after(Action a) {
        using enum Action;
        using CV = CommitVerifier;
        switch (a) {
            case AddMessage:
                return set(&CV::has_message, true);

            case AddFolder:
            case AddFile:
                return set(&CV::has_adds, true);

            case AddAll:
                return set(&CV::has_adds, true) | set(&CV::has_added_all, true);

            case SetTimestamp:
            case SetTimestampNow:
                return set(&CV::has_timestamp, true);

            case SetAuthor:
            case SetAuthorSelf:
            case SetMultipleAuthors:
                return set(&CV::has_author, true);

            case SetFixup:
            case SetSquash:
                return set(&CV::has_fixup_or_squash, true);
        }
    }
};

template <CommitVerifier V>
struct CommitBuilder : private magic_bldr::Builder<Commit, CommitVerifier, CommitData, V> {
    auto&& add_message(std::string&& msg) && {
        return MB::run<MB::Action::AddMessage>(std::move(msg));
    }

    auto&& add_folder(std::filesystem::path&& folder_path) && {
        return MB::run<MB::Action::AddFolder>(std::move(folder_path));
    }

    auto&& add_file(std::filesystem::path&& file_path) && {
        return MB::run<MB::Action::AddFile>(std::move(file_path));
    }

    auto&& add_all() && {
        return MB::run<MB::Action::AddAll>({});
    }

    auto&& set_timestamp(std::chrono::time_point<std::chrono::system_clock>&& ts) && {
        return MB::run<MB::Action::SetTimestamp>(ts);
    }

    auto&& set_timestamp_now() && {
        return MB::run<MB::Action::SetTimestampNow>({});
    }

    auto&& set_author_self() && {
        return MB::run<MB::Action::SetAuthorSelf>({});
    }

    auto&& set_author(std::string&& author) && {
        return MB::run<MB::Action::SetAuthor>(std::move(author));
    }

    template <std::convertible_to<std::string>... Ts>
    auto&& set_multiple_authors(Ts&&... authors) && {
        return MB::run<MB::Action::SetAuthor>(std::move(authors), ...);
    }

    auto&& set_fixup_hash(std::string&& hash) && {
        return MB::run<MB::Action::SetFixup>(std::move(hash));
    }

    auto&& set_squash_hash(std::string&& hash) && {
        return MB::run<MB::Action::SetFixup>(std::move(hash));
    }

  private: 
    using MB = magic_bldr::Builder<Commit, CommitVerifier, CommitData, V>;
};

int main() {}