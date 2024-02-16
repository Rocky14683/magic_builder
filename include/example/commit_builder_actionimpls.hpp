#pragma once

#include "commit_builder.hpp"

template <magic_bldr::TemplateStringLiteral Reason>
using CVWithFailReason = CommitVerifier::FailureReasonWrapper<Reason>;

template <>
struct magic_bldr::ActionImpl<CommitVerifier::Action::AddAll> {
    static constexpr void run(CommitData* d) {
        d->included_paths.emplace_back("*");
    }

    static consteval bool is_allowed(CommitVerifier v) {
        return !v.has_added_all && !v.has_adds;
    }
};

template <typename CommitVerifier::Action A>
    requires (magic_bldr::is_in(A, {CommitVerifier::Action::AddFile,
                                   CommitVerifier::Action::AddFolder}))
struct magic_bldr::ActionImpl<A> {
    // Actions can share one ActionImpl function without necessarily sharing
    // the other one (disambiguate in the non-shared one via `if constexpr`)

    static constexpr void run(CommitData* d, std::filesystem::path&& path) {
        if constexpr (A == CommitVerifier::Action::AddFile) {
            d->included_paths.emplace_back(std::move(path));
        } else {
            d->included_paths.emplace_back(std::move(path));
        }
    }

    static consteval bool is_allowed(CommitVerifier v) {
        return !v.has_added_all;
    }
};

template <typename CommitVerifier::Action A>
    requires (magic_bldr::is_in(A, {CommitVerifier::Action::SetTimestamp,
                                    CommitVerifier::Action::SetTimestampNow}))
struct magic_bldr::ActionImpl<A> {
    // Actions with different impls can also share one ActionImpl function
    // through constrained overloads.

    static constexpr void run(CommitData* d, std::chrono::time_point<std::chrono::system_clock>&& ts)
    requires (A == CommitVerifier::Action::SetTimestamp) {
        d->timestamp = std::move(ts);
    }

    static constexpr void run(CommitData* d)
    requires (A == CommitVerifier::Action::SetTimestampNow) {
        d->timestamp = std::chrono::system_clock::now();
    }

    static consteval bool is_allowed(CommitVerifier v) { return !v.has_timestamp; }
};

template <typename CommitVerifier::Action A>
    requires (magic_bldr::is_in(A, {CommitVerifier::Action::SetAuthor,
                                   CommitVerifier::Action::SetAuthorSelf,
                                   CommitVerifier::Action::SetMultipleAuthors}))
struct magic_bldr::ActionImpl<A> {
    using enum CommitVerifier::Action;

    static constexpr void run(CommitData* d, std::string&& author)
    requires (A == SetAuthor) {
        d->authors.emplace_back(std::move(author));
    }

    static constexpr void run(CommitData* d)
    requires (A == SetAuthorSelf) {
        // Toy implementation, presumably you'd have the "Self" credential
        // somewhere in a real version of this git wrapper.
        d->authors.emplace_back("me!");
    }

    static constexpr void run(CommitData* d, std::vector<std::string>&& authors)
    requires (A == SetMultipleAuthors) {
        d->authors = std::move(authors);
    }

    static consteval bool is_allowed(CommitVerifier v) { return !v.has_author; }
};

