/* SPDX-License-Identifier: MIT */
/*
 * v_motor_base.h - 垂直电机基类（直流电机抽象）。
 *
 * 3 个虚方法 + 2 个回调，覆盖直流电机能做的全部事情：
 *
 *     motor_stop / motor_move / fault_clear   ← ops 表
 *     v_motor_move_status_changed_cb          ← 状态变化回调
 *     v_motor_err_cb_t                        ← 硬件错误回调
 *
 * 为什么只 3 个虚方法？因为有刷直流电机本身能力域窄：
 *     - 没有"绝对定位"概念（没编码器闭环）
 *     - 没有"加减速曲线参数"概念（驱动 IC 内部固定）
 *     - 没有"I²t / 跟随误差 / 限位编码器" 这一套
 * 能做的就是：开（往哪个方向转）/关/清故障。
 *
 * 这跟 h_motor_base 的 24 个虚方法形成强烈对照, 证明 ops 表的规模严格匹
 * 配硬件能力，不是"越多越好"也不是"为了对称勉强凑数"。
 */

#ifndef __V_MOTOR_BASE_H
#define __V_MOTOR_BASE_H

#include <stdbool.h>

/* 运动方向 / 停止状态。
 * 注意 STOP 和 UP/DOWN 是同一个 enum：方便 move_state 字段统一
 * 表达"当前不动"vs"当前往上/下"。 */
enum v_motor_base_dir {
	V_MOTOR_BASE_MOVE_DOWN = 0,
	V_MOTOR_BASE_MOVE_UP,
	V_MOTOR_BASE_MOVE_STOP,
};

/* 错误码。负值，方便 err_cb 同时表达"哪类错"。
 * 当前只一种 HW 故障；将来扩展（通信丢、看门狗超时等）
 * 在这里加新枚举值，应用层 err_cb 内部 switch 即可。 */
enum v_motor_base_err_id {
	V_MOTOR_HW_ERR = -1,
};

struct v_motor_base;

/* 错误回调签名：err_id 标识哪类错，me 指向出错的电机实例
 * （多个电机共享同一个 err_cb 时，靠 me 区分来源）。
 *
 * callback typedef 是 C 工程标准做法：函数指针类型不是数据 struct，
 * typedef 成简短名字让上层注册函数签名干净。 */
typedef void (*v_motor_err_cb_t)(enum v_motor_base_err_id err_id,
                                 struct v_motor_base *me);

/* 3 个虚方法的 ops 表。子类（IFX007T / 其他 H 桥 IC）实现
 * 一份静态 ops，挂到 struct v_motor_base.ops 字段上。 */
struct v_motor_base_ops {
	/* 立即停车。各子类按自己硬件方案实现"停"的具体动作
	 * （断 PWM、INH 拉低、刹车短路 ...）。 */
	void (*motor_stop)(struct v_motor_base *me);

	/* 向 dir 方向运动。子类负责加减速曲线、软启动、限位检查。 */
	void (*motor_move)(struct v_motor_base *me,
	                   enum v_motor_base_dir dir);

	/* 清除硬件故障锁（IFX007T 过温自锁等）。可空，
	 * 不实现就用 if (ops->fault_clear) 在基类层兜底。 */
	void (*fault_clear)(struct v_motor_base *me);
};

/* 状态变化回调签名：电机方向从 X 切到 Y 时被调一次。
 * 应用层用它做"启动/停止"音效、点亮/熄灭指示灯等。 */
typedef void (*v_motor_move_status_changed)(
	struct v_motor_base *me,
	enum v_motor_base_dir status);

struct v_motor_base {
	/* 指向静态 ops 表（每个子类一份共享 const）。 */
	const struct v_motor_base_ops *ops;

	/* 当前运动状态（STOP / UP / DOWN）。
	 * 由基类封装函数维护，避免子类要各自记一份。 */
	enum v_motor_base_dir move_state;

	/* 状态变化回调。NULL 表示应用层不关心这事件。 */
	v_motor_move_status_changed v_motor_move_status_changed_cb;

	/* 硬件错误回调。子类内部检测到故障时回调到上层。 */
	v_motor_err_cb_t err_cb;
};


/* 基类公开 API
 *
 * 应用层只调这一组函数，永远看不到 ops 字段。函数体内部走
 * me->ops->xxx(me, ...) 派发到具体子类实现。 */

void v_motor_init(struct v_motor_base *me);

void v_motor_base_move(struct v_motor_base *me,
                       enum v_motor_base_dir dir);

void v_motor_base_stop(struct v_motor_base *me);

enum v_motor_base_dir v_motor_base_status_get(struct v_motor_base *me);

void v_motor_move_status_change_cb_register(
	struct v_motor_base *me,
	v_motor_move_status_changed cb);

void v_motor_base_fault_cb_register(
	struct v_motor_base *me,
	v_motor_err_cb_t cb);

void v_motor_base_fault_clear(struct v_motor_base *me);

#endif /* __V_MOTOR_BASE_H */
