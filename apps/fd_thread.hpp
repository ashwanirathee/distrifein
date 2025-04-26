#ifndef FD_THREAD_HPP
#define FD_THREAD_HPP

#pragma once
#include <atomic>
#include <distrifein/logger.hpp>

void failure_detector_thread(std::atomic<bool> &running, Logger &logger);

#endif