/* SPDX-License-Identifier: MIT */
/* syntax_stubs/gpiod.h - libgpiod 1.x 最小占位声明 (仅供 make check-syntax 用) */

#ifndef SYNTAX_STUBS_GPIOD_H_
#define SYNTAX_STUBS_GPIOD_H_

struct gpiod_chip;
struct gpiod_line;

struct gpiod_chip *gpiod_chip_open_by_name(const char *name);
void               gpiod_chip_close(struct gpiod_chip *chip);
struct gpiod_line *gpiod_chip_get_line(struct gpiod_chip *chip,
                                       unsigned int offset);
int                gpiod_line_request_output(struct gpiod_line *line,
                                             const char *consumer,
                                             int default_val);
int                gpiod_line_set_value(struct gpiod_line *line, int value);
void               gpiod_line_release(struct gpiod_line *line);

#endif /* SYNTAX_STUBS_GPIOD_H_ */
