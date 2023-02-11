#include "fetchers.hh"
#include "input-accessor.hh"
#include "store-api.hh"
#include <gtest/gtest.h>
#include <git2/commit.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/index.h>
#include <git2/repository.h>
#include <git2/signature.h>
#include <git2/tree.h>
#include <gmock/gmock-matchers.h>

using namespace nix;
using namespace nix::fetchers;

using testing::SizeIs;

class TmpDir {
public:
    const nix::Path dir;
private:
    AutoDelete autoDelete;

public:
    TmpDir() : dir(createTempDir()), autoDelete(dir) { }
};

class TmpStore : public TmpDir {
    ref<Store> store;
public:
    TmpStore() : store(openStore(dir)) { }

    operator ref<Store>() {
        return store;
    }
};

class TmpRepo : public TmpDir {
    AutoDelete autoDelete;

public:
    git_repository *repo = NULL;

    TmpRepo(bool isBare = false)
    {
        git_libgit2_init();
        if (git_repository_init(&repo, dir.c_str(), isBare))
            throw Error("creating Git repository '%s': %s", dir, git_error_last()->message);
    }
    ~TmpRepo() {
        git_repository_free(repo);
        git_libgit2_shutdown();
    }

    // For troubleshooting a test
    void dump() {
        (void) system((std::string("exec 2>&1; cd '") + dir + "'; (set -eux; ls -alR; git reflog; git log; git log --graph --oneline --branches --all --color=always; git status;) | cat").c_str());
    }
};

static void commitIndexNoParent(git_repository *repo, git_oid &tree_id, git_oid &commit_id) {
    git_signature *sig;
	git_tree *tree;

    git_signature_now(&sig, "Test User", "test@example.com");

	if (git_tree_lookup(&tree, repo, &tree_id) < 0)
		throw Error("Could not look up initial tree: %s", git_error_last()->message);

	if (git_commit_create_v(
			&commit_id, repo, "HEAD", sig, sig,
			NULL, "Initial commit", tree, 0) < 0)
		throw Error("Could not create the initial commit: %s", git_error_last()->message);

	git_tree_free(tree);
	git_signature_free(sig);
}

static void initCommit(git_repository *repo) {
	git_index *index;
	git_oid tree_id, commit_id;

    if (git_repository_index(&index, repo) < 0)
		throw Error("Could not open repository index: %s", git_error_last()->message);

	if (git_index_write_tree(&tree_id, index) < 0)
		throw Error("Unable to write initial tree from index: %s", git_error_last()->message);

	git_index_free(index);

    commitIndexNoParent(repo, tree_id, commit_id);
}

// I think an empty source would be ok?
TEST(git, when_the_repo_and_worktree_are_empty__fetchTree_on_the_worktree_returns_an_error) {
    TmpRepo repo;
    TmpStore store;

    Attrs attrs;
    attrs.emplace("type", "git");
    attrs.emplace("url", std::string("file://") + repo.dir);

    auto input = Input::fromAttrs(std::move(attrs));
    auto r = input.getAccessor(store);

    InputAccessor &accessor = *r.first;

    ASSERT_THROW({
        auto root = accessor.root();
        auto dir = root.readDirectory();
    }, RestrictedPathError);
}

// I think an empty source would be ok?
TEST(git, when_the_repo_head_and_worktree_are_empty__fetchTree_on_the_worktree_returns_an_error) {
    TmpRepo repo;
    TmpStore store;

    initCommit(repo.repo);

    Attrs attrs;
    attrs.emplace("type", "git");
    attrs.emplace("url", std::string("file://") + repo.dir);

    auto input = Input::fromAttrs(std::move(attrs));
    auto r = input.getAccessor(store);

    InputAccessor &accessor = *r.first;

    ASSERT_THROW({
        auto root = accessor.root();
        auto dir = root.readDirectory();
    }, RestrictedPathError);
}

TEST(git, when_the_repo_head_exists_empty_and_worktree_are_empty__fetchTree_on_the_head_returns_an_empty_tree) {
    TmpRepo repo;
    TmpStore store;

    initCommit(repo.repo);

    Attrs attrs;
    attrs.emplace("type", "git");
    attrs.emplace("ref", "HEAD");
    attrs.emplace("url", std::string("file://") + repo.dir);

    auto input = Input::fromAttrs(std::move(attrs));
    auto r = input.getAccessor(store);

    InputAccessor &accessor = *r.first;

    auto root = accessor.root();
    auto dir = root.readDirectory();
    ASSERT_THAT(dir, SizeIs(0));

    auto fetched = input.fetchToStore(store);
    ASSERT_EQ(fetched.first.to_string(), "0ccnxa25whszw7mgbgyzdm4nqc0zwnm8-source");
}

TEST(git, when_the_head_contains_a_file_and_head_is_requested_return_the_file) {
    TmpRepo repo;
    TmpStore store;


	git_index *index;
	git_oid tree_id, commit_id;

    if (git_repository_index(&index, repo.repo) < 0)
		throw Error("Could not open repository index: %s", git_error_last()->message);

    git_index_entry index_entry;
    memset(&index_entry, 0, sizeof(index_entry));
    index_entry.path = "hello";
    index_entry.mode = GIT_FILEMODE_BLOB;

    if (git_index_add_from_buffer(index, &index_entry, "hi", 2))
        throw Error("Could not add to index: %s", git_error_last()->message);

	if (git_index_write_tree(&tree_id, index) < 0)
		throw Error("Unable to write initial tree from index: %s", git_error_last()->message);

	git_index_free(index);

    commitIndexNoParent(repo.repo, tree_id, commit_id);

    Attrs attrs;
    attrs.emplace("type", "git");
    attrs.emplace("ref", "HEAD");
    attrs.emplace("url", std::string("file://") + repo.dir);

    auto input = Input::fromAttrs(std::move(attrs));
    auto r = input.getAccessor(store);

    InputAccessor &accessor = *r.first;

    auto root = accessor.root();
    auto dir = root.readDirectory();

    ASSERT_THAT(dir, SizeIs(1));

    auto fetched = input.fetchToStore(store);
    ASSERT_EQ(fetched.first.to_string(), "5rbhbvyxfxgc6ac9f65qqa7q193qanls-source");
}
