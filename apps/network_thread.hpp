#ifndef NETWORK_THREAD_HPP
#define NETWORK_THREAD_HPP

#pragma once
#include <atomic>
#include <distrifein/logger.hpp>

void network_thread(std::atomic<bool>& running, Logger &logger);

#endif
