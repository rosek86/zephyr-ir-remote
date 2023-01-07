#ifndef UTILS_H__
#define UTILS_H__

#define K_MSGQ_DEFINE_STATIC(q_name, q_msg_size, q_max_msgs, q_align) \
  static char __noinit __aligned(q_align)                             \
    _k_fifo_buf_##q_name[(q_max_msgs) * (q_msg_size)];                \
  static STRUCT_SECTION_ITERABLE(k_msgq, q_name) =                    \
         Z_MSGQ_INITIALIZER(q_name, _k_fifo_buf_##q_name,             \
          q_msg_size, q_max_msgs)

#endif // UTILS_H__
