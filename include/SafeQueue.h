/** 
 * @file SafeQueue.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/5
 * @description 泛型线程安全队列模板类
 */
#ifndef MNSX_KRAKENPOOL_SAFEQUEUE_H
#define MNSX_KRAKENPOOL_SAFEQUEUE_H

#include <limits>
#include <mutex>
#include <queue>

namespace mnsx {
    namespace kraken {

        template <typename T>
        class SafeQueue {
        public:
            /**
             * @brief 构造函数
             * @param max_size 容器最大容量，默认设置DEFAULT_MAC_SIZE
             */
            explicit SafeQueue(size_t max_size = DEFAULT_MAX_SIZE) : max_size_(max_size) {};

            /**
             * @brief 析构函数
             */
            ~SafeQueue() = default;

            // 因为模板类携带锁，禁止拷贝和赋值
            SafeQueue(const SafeQueue&) = delete;
            SafeQueue& operator=(const SafeQueue&) = delete;

            /**
             * @brief 将一个对象拷贝或移动进队列中
             * @param new_value 需要加入队列的对象
             */
            bool push(T new_value) {

                std::lock_guard<std::mutex> lock(this->mutex_);

                // 容量已满不允许添加
                if (this->queue_.size() >= this->max_size_) {
                    return false;
                }

                this->queue_.push(std::move(new_value));
                return true;
            }

            /**
             * @brief 将对象的组成参数传入队列，在队列内部构造
             * @tparam Args 入队对象的构造参数类型包
             * @param args 入队对象的构造参数
             */
            template <typename... Args>
            bool emplace(Args&&... args) {

                std::lock_guard<std::mutex> lock(this->mutex_);

                // 容量已满不允许添加
                if (this->queue_.size() >= this->max_size_) {
                    return false;
                }

                this->queue_.emplace(std::forward<Args>(args)...);
                return true;
            }

            /**
             * @brief 非阻塞的将队列中的对象移动到返回值引用
             * @param value 返回值引用
             * @return 返回是否出队成功
             */
            bool tryPop(T& value) {
                std::lock_guard<std::mutex> lock(this->mutex_);
                if (this->queue_.empty()) {
                    return false;
                }

                // 压榨效率，因为下一行pop，所以直接移动
                value = std::move(const_cast<T&>(this->queue_.top()));
                this->queue_.pop();
                return true;
            }

            /**
             * @brief 检查队列是否为空
             * @return 队列是否为空
             */
            bool empty() const {
                std::lock_guard<std::mutex> lock(this->mutex_);
                return this->queue_.empty();
            }

            /**
             * @brief 获取当前队列大小
             * @return 当前队列大小
             */
            size_t size() const {
                std::lock_guard<std::mutex> lock(this->mutex_);
                return this->queue_.size();
            }

            /**
             * @brief 暴露底层互斥锁的引用，方便外部使用（慎用）
             * @return 底层互斥锁引用
             */
            std::mutex& getMutex() {
                return this->mutex_;
            }

            /**
             * @brief 暴露底层队列（慎用，配合 getMutex() 保证安全）
             * @return 底层队列引用
             */
            std::priority_queue<T>& getUnderlyingQueue() {
                return this->queue_;
            }

        private:
            mutable std::mutex mutex_; // 互斥锁
            std::priority_queue<T> queue_; // 队列容器

            size_t max_size_; // 队列最大容量
            constexpr const static size_t DEFAULT_MAX_SIZE = std::numeric_limits<size_t>::max(); // 默认队列最大容量
        };

    }
}


#endif //MNSX_KRAKENPOOL_SAFEQUEUE_H