#include "../../include/asm/io.h"
#include "../../include/linux/tty.h"
#include "../../include/string.h"

#define CRT_ADDR_REG 0x3D4 // CRT(6845) index register
#define CRT_DATA_REG 0x3D5 // CRT(6845) data register

#define CRT_START_ADDR_H 0xC // memory start locaiton-- high
#define CRT_START_ADDR_L 0xD // memory start locaiton-- low
#define CRT_CURSOR_H 0xE     // cursor high
#define CRT_CURSOR_L 0xF     // cursor low

#define MEM_BASE 0xB8000              // graphic card start location
#define MEM_SIZE 0x4000               // graphic card memory size
#define MEM_END (MEM_BASE + MEM_SIZE) // graphic card ending locaiton
#define WIDTH 80                      // screen col
#define HEIGHT 25                     // screen row
#define ROW_SIZE (WIDTH * 2)          // # of bytes/row
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // # pf bytes/screen

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07 // \a
#define ASCII_BS 0x08  // \b
#define ASCII_HT 0x09  // \t
#define ASCII_LF 0x0A  // \n
#define ASCII_VT 0x0B  // \v
#define ASCII_FF 0x0C  // \f
#define ASCII_CR 0x0D  // \r
#define ASCII_DEL 0x7F

static uint screen;
static uint pos;
static uint x, y;

// set screen start location
static void set_screen() {
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_H);
    out_byte(CRT_DATA_REG, ((screen - MEM_BASE) >> 9) & 0xff);
    out_byte(CRT_ADDR_REG, CRT_START_ADDR_L);
    out_byte(CRT_DATA_REG, ((screen - MEM_BASE) >> 1) & 0xff);
}

static void set_cursor()
{
    out_byte(CRT_ADDR_REG, CRT_CURSOR_H);
    out_byte(CRT_DATA_REG, ((pos - MEM_BASE) >> 9) & 0xff);
    out_byte(CRT_ADDR_REG, CRT_CURSOR_L);
    out_byte(CRT_DATA_REG, ((pos - MEM_BASE) >> 1) & 0xff);
}

void console_clear()
{
    screen = MEM_BASE;
    pos = MEM_BASE;
    x = y = 0;
    set_cursor();
    set_screen();

    u16 *ptr = (u16 *)MEM_BASE;
    while (ptr < MEM_END)
    {
        *ptr++ = 0x0720;
    }
}


static void scroll_up()
{
    if (screen + SCR_SIZE + ROW_SIZE < MEM_END)
    {
        u32 *ptr = (u32 *)(screen + SCR_SIZE);
        for (size_t i = 0; i < WIDTH; i++)
        {
            *ptr++ = 0x0720;
        }
        screen += ROW_SIZE;
        pos += ROW_SIZE;
    }
    else
    {
        memcpy(MEM_BASE, screen, SCR_SIZE);
        pos -= (screen - MEM_BASE);
        screen = MEM_BASE;
    }
    set_screen();
}

static void command_lf()
{
    if (y + 1 < HEIGHT)
    {
        y++;
        pos += ROW_SIZE;
        return;
    }
    scroll_up();
}

static void command_cr()
{
    pos -= (x << 1);
    x = 0;
}

static void command_bs()
{
    if (x)
    {
        x--;
        pos -= 2;
        *(u16 *)pos = 0x0720;
    }
}

static void command_del()
{
    *(u16 *)pos = 0x0720;
}

void console_write(char *buf, u32 count)
{
    char ch;
    char *ptr = (char *)pos;
    while (count--)
    {
        ch = *buf++;
        switch (ch)
        {
            case ASCII_NUL:
                break;
            case ASCII_BEL:
                break;
            case ASCII_BS:
                command_bs();
                break;
            case ASCII_HT:
                break;
            case ASCII_LF:
                command_lf();
                command_cr();
                break;
            case ASCII_VT:
                break;
            case ASCII_FF:
                command_lf();
                break;
            case ASCII_CR:
                command_cr();
                break;
            case ASCII_DEL:
                command_del();
                break;
            default:
                if (x >= WIDTH)
                {
                    x -= WIDTH;
                    pos -= ROW_SIZE;
                    command_lf();
                }

                *ptr = ch;
                ptr++;
                *ptr = 0x07;
                ptr++;

                pos += 2;
                x++;
                break;
        }
    }
    set_cursor();
}

void console_init(void) {
    console_clear();
}