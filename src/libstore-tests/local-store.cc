#include <gtest/gtest.h>

#include <future>

#include "nix/store/globals.hh"
#include "nix/store/local-store.hh"
#include "nix/store/pathlocks.hh"
#include "nix/store/store-open.hh"
#include "nix/util/file-system.hh"

// Needed for template specialisations. This is not good! When we
// overhaul how store configs work, this should be fixed.
#include "nix/util/args.hh"
#include "nix/util/config-impl.hh"
#include "nix/util/abstract-setting-to-json.hh"

namespace nix {

TEST(LocalStore, storeDir_absolutePath)
{
    std::filesystem::path storeDir =
#ifdef _WIN32
        "C:\\";
#else
        "/";
#endif
    storeDir /= "nix";
    storeDir /= "store";
    LocalStoreConfig config{"", {{"store", storeDir.string()}}};
    EXPECT_EQ(config.storeDir, storeDir.string());
}

TEST(LocalStore, storeDir_relativePath_rejected)
{
    EXPECT_THROW(LocalStoreConfig("", {{"store", (std::filesystem::path{"nix"} / "store").string()}}), UsageError);
}

TEST(LocalStore, storeDir_empty_rejected)
{
    EXPECT_THROW(LocalStoreConfig("", {{"store", ""}}), UsageError);
}

TEST(LocalStore, constructConfig_rootQueryParam)
{
#ifdef _WIN32
    constexpr std::string_view root = "C:\\foo\\bar";
#else
    constexpr std::string_view root = "/foo/bar";
#endif
    LocalStoreConfig config{
        "",
        {
            {
                "root",
                std::string{root},
            },
        },
    };

    EXPECT_EQ(config.rootDir.get(), std::optional<AbsolutePath>{std::string{root}});
}

TEST(LocalStore, constructConfig_rootPath)
{
#ifdef _WIN32
    constexpr std::string_view root = "C:\\foo\\bar";
#else
    constexpr std::string_view root = "/foo/bar";
#endif
    LocalStoreConfig config{std::string{root}, {}};

    EXPECT_EQ(config.rootDir.get(), std::optional<AbsolutePath>{std::string{root}});
}

TEST(LocalStore, constructConfig_to_string)
{
    LocalStoreConfig config{"", {}};
    EXPECT_EQ(config.getReference().to_string(), "local");
}

#ifndef _WIN32

TEST(LocalStore, autoGCDoesNotWaitForGCLock)
{
    auto tmpRoot = createTempDir();
    createDirs(tmpRoot / "nix/store");

    auto store = openStore(fmt("local?root=%s", tmpRoot.string())).dynamic_pointer_cast<LocalStore>();
    ASSERT_NE(store, nullptr);

    auto & gcSettings = settings.getLocalSettings().getGCSettings();
    auto oldMinFree = gcSettings.minFree.get();
    auto oldMaxFree = gcSettings.maxFree.get();
    auto oldMinFreeCheckInterval = gcSettings.minFreeCheckInterval.get();
    Finally restoreSettings([&]() {
        gcSettings.minFree = oldMinFree;
        gcSettings.maxFree = oldMaxFree;
        gcSettings.minFreeCheckInterval = oldMinFreeCheckInterval;
    });
    gcSettings.minFree = std::numeric_limits<uint64_t>::max() - 1;
    gcSettings.maxFree = std::numeric_limits<uint64_t>::max();
    gcSettings.minFreeCheckInterval = 0;

    std::future<void> autoGC;
    std::future_status status;
    {
        auto gcLockFile = openLockFile(tmpRoot / "nix/var/nix/gc.lock", true);
        FdLock gcLock(gcLockFile.get(), ltWrite, true, "");

        autoGC = std::async(std::launch::async, [&]() { store->autoGC(); });
        status = autoGC.wait_for(std::chrono::seconds(2));
    }
    autoGC.get();

    EXPECT_EQ(status, std::future_status::ready);
}

#endif

} // namespace nix
