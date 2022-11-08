// Run using `env -u WAYLAND_DISPLAY xvfb-run ./ubuntu_frame_test`

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>

#include <memory>

using namespace testing;

namespace
{
struct Process
{
    explicit Process(pid_t pid) : pid{pid} {}
    ~Process() { kill(pid, SIGTERM); }

    pid_t const pid;
};

struct UbuntuFrameTest : public testing::Test
{
    UbuntuFrameTest();

    ~UbuntuFrameTest();

    auto pixel_color(int x, int y) -> uint32_t;

protected:
    void SetUp() override;

    void TearDown() override;

private:
    Display* const display;
    pid_t frame_pid = 0;
};

pid_t spawn(char const* cmd)
{
    if (int const pid = fork())
    {
        return pid;
    }
    else
    {
        // Silence stdout/stderr
        close(1);
        close(2);
        char* argv[] = {const_cast<char*>(cmd), nullptr};

        execv(cmd, argv);
        perror(cmd);
        exit(errno);
    }
}

auto UbuntuFrameTest::pixel_color(int x, int y) -> uint32_t
{

    std::unique_ptr<XImage, void(*)(XImage*)> const image{
        XGetImage(display, XRootWindow(display, XDefaultScreen(display)), x, y, 1, 1, AllPlanes, XYPixmap),
        [](XImage* image) { XFree(image); }};

    XColor c;
    c.pixel = XGetPixel(image.get(), 0, 0);

    XQueryColor(display, XDefaultColormap(display, XDefaultScreen(display)), &c);

    return c.pixel;
}

UbuntuFrameTest::UbuntuFrameTest() :
    display{XOpenDisplay((char*)NULL)}
{
    if (display == nullptr)
        exit(1);

    setenv("WAYLAND_DISPLAY", "wayland-97", 1);
}

UbuntuFrameTest::~UbuntuFrameTest()
{
    XCloseDisplay(display);
}

void UbuntuFrameTest::SetUp()
{
    Test::SetUp();

    frame_pid = spawn("/snap/bin/ubuntu-frame");
}

void UbuntuFrameTest::TearDown()
{
    kill(frame_pid, SIGTERM);
    Test::TearDown();
}

TEST_F(UbuntuFrameTest, AtStartFrameShowsGreyscale)
{
    sleep(2);
    EXPECT_THAT(pixel_color(10, 10), Eq(0x0A0A0A));
}

TEST_F(UbuntuFrameTest, After5SFrameShowsDiagnostic)
{
    sleep(7);
    EXPECT_THAT(pixel_color(10, 10), Eq(0x060304));
}

TEST_F(UbuntuFrameTest, AfterAppStartsFrameShowsApp)
{
    sleep(2);
    Process gedit{spawn("/usr/bin/gedit")};
    sleep(2);
    EXPECT_THAT(pixel_color(10, 10), Ne(0x0A0A0A));
    EXPECT_THAT(pixel_color(10, 10), Ne(0x060304));
}

TEST_F(UbuntuFrameTest, AfterAppExitsFrameShowsGreyscale)
{
    sleep(2);
    {
        Process gedit{spawn("/usr/bin/gedit")};
        sleep(2);
    }
    sleep(2);
    EXPECT_THAT(pixel_color(10, 10), Eq(0x0A0A0A));
}

TEST_F(UbuntuFrameTest, AfterAppExitsAndTimeoutFrameShowsDiagnostic)
{
    sleep(2);
    {
        Process gedit{spawn("/usr/bin/gedit")};
        sleep(2);
    }
    sleep(7);
    EXPECT_THAT(pixel_color(10, 10), Eq(0x060304));
}
}
