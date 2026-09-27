P = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8/src/render/Renderer.cpp"
s = open(P).read()
def rep(old, new):
    global s
    assert s.count(old) == 1, old[:80]
    s = s.replace(old, new)

rep("""    std::promise<bool> promise;
    auto future = promise.get_future();
""", """    std::promise<CaptureRead> promise;
    auto future = promise.get_future();
""")

rep("""    if (!future.get())
        return false;

    // s-rta-0927 plan-renderperf C3: the GL thread read the canvas (processPendingCapture) and went on rendering;
    // the conversion, the PNG encode and the file write happen here, on this already-waiting thread. Same pixels,
    // same bytes (PixelConvert == the old setPixelColour loop), and the file is complete before the return (the
    // stream's scope closes -- and flushes -- before the log line).
    std::vector<uint8_t> pixels;
    int readW = 0, readH = 0;
    double readMs = 0.0;
    {
        std::lock_guard<std::mutex> lock(captureMutex_);
        pixels = std::move(capturePixels_);
        capturePixels_ = {};
        readW = captureReadW_;
        readH = captureReadH_;
        readMs = captureReadMs_;
    }
    if (readW <= 0 || readH <= 0 || pixels.size() != static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4)
""", """    // s-rta-0927 plan-renderperf C3: the GL thread read the canvas (processPendingCapture) and went on rendering;
    // the conversion, the PNG encode and the file write happen here, on this already-waiting thread. Same pixels,
    // same bytes (PixelConvert == the old setPixelColour loop), and the file is complete before the return (the
    // stream's scope closes -- and flushes -- before the log line). The read arrives by value through THIS call's
    // own future (fix round): nothing shared is read after the signal, so a second capture armed and serviced
    // before this line cannot hand its pixels to this caller.
    CaptureRead read = future.get();
    if (!read.ok)
        return false;
    std::vector<uint8_t>& pixels = read.pixels;
    const int readW = read.width;
    const int readH = read.height;
    const double readMs = read.readMs;
    if (readW <= 0 || readH <= 0 || pixels.size() != static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4)
""")

rep("""        std::cerr << "[Eyes] Invalid capture dimensions: " << readW << "x" << readH << std::endl;
        capturePromise_->set_value(false);
""", """        std::cerr << "[Eyes] Invalid capture dimensions: " << readW << "x" << readH << std::endl;
        capturePromise_->set_value(CaptureRead{});
""")

rep("""    const auto tRead = std::chrono::steady_clock::now();
    capturePixels_.resize(static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
    glReadPixels(0, 0, readW, readH, GL_RGBA, GL_UNSIGNED_BYTE, capturePixels_.data());
    captureReadMs_ = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tRead).count();
    captureReadW_ = readW;
    captureReadH_ = readH;

    capturePromise_->set_value(true);
""", """    const auto tRead = std::chrono::steady_clock::now();
    CaptureRead read;
    read.pixels.resize(static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
    glReadPixels(0, 0, readW, readH, GL_RGBA, GL_UNSIGNED_BYTE, read.pixels.data());
    read.readMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tRead).count();
    read.width = readW;
    read.height = readH;
    read.ok = true;

    // Handed to this request's own future by value -- never parked in a Renderer member (fix round).
    capturePromise_->set_value(std::move(read));
""")
open(P, "w").write(s)
print("ok")
