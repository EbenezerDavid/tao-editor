/* !/usr/bin/gcc

GNU GENERAL PUBLIC LICENSE
2025/3/13    EbenezerDavid

*/


#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h> // 引入 GtkSourceView 头文件
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 100

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
} FileBuffer;

typedef struct {
    FileBuffer fb;
    char filename[777];
    GtkSourceBuffer *text_buffer; // 改为 GtkSourceBuffer 以支持撤销
    GtkWidget *status_label;
    int mode;  // 0=COMMAND, 1=INSERT
    char cmd_buf[100];
    int cmd_len;
    int cmd_active;
} EditorState;

// 加载文件到缓冲区
void load_file(const char *filename, FileBuffer *fb) {
    FILE *fp = fopen(filename, "r");
    fb->line_count = 0;

    if (fp == NULL) {
        fp = fopen(filename, "w");
        if (fp == NULL) {
            printf("Can't create file: %s\n", filename);
            exit(EXIT_FAILURE);
        }
        fclose(fp);
        return;
    }

    while (fb->line_count < MAX_LINES && fgets(fb->lines[fb->line_count], MAX_LINE_LEN, fp) != NULL) {
        fb->lines[fb->line_count][strcspn(fb->lines[fb->line_count], "\n")] = '\0';
        fb->line_count++;
    }
    fclose(fp);
}

// 保存缓冲区到文件
void save_file(const char *filename, FileBuffer *fb) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Can't save file: %s\n", filename);
        return;
    }

    for (int i = 0; i < fb->line_count; i++) {
        fprintf(fp, "%s\n", fb->lines[i]);
    }
    fclose(fp);
    printf("Saved %d lines to %s\n", fb->line_count, filename);
}

// 更新文本缓冲区（显示用）
void update_text_buffer(EditorState *state) {
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(state->text_buffer), "", -1);
    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(state->text_buffer), &iter);

    for (int i = 0; i < state->fb.line_count; i++) {
        char line[256];
        snprintf(line, sizeof(line), "%3d | %s\n", i + 1, state->fb.lines[i]);
        gtk_text_buffer_insert(GTK_TEXT_BUFFER(state->text_buffer), &iter, line, -1);
    }
}

// 从文本缓冲区更新FileBuffer（保存用）
void sync_file_buffer(EditorState *state) {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(state->text_buffer), &start, &end);
    char *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(state->text_buffer), &start, &end, FALSE);

    state->fb.line_count = 0;
    char *line = strtok(text, "\n");
    while (line && state->fb.line_count < MAX_LINES) {
        char *content = strstr(line, "| ");
        if (content) {
            content += 2;
        } else {
            content = line;
        }
        strncpy(state->fb.lines[state->fb.line_count], content, MAX_LINE_LEN - 1);
        state->fb.lines[state->fb.line_count][MAX_LINE_LEN - 1] = '\0';
        state->fb.line_count++;
        line = strtok(NULL, "\n");
    }
    g_free(text);
}

// 处理按键（添加 Ctrl + Z 撤销）
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, EditorState *state) {
    // 检测 Ctrl + Z（撤销）
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_z) {
        if (state->mode == 1 && gtk_source_buffer_can_undo(state->text_buffer)) { // 仅在 INSERT 模式下撤销
            gtk_source_buffer_undo(state->text_buffer);
            return TRUE;
        }
    }

    if (event->keyval == GDK_KEY_Escape) {
        state->mode = 0;
        state->cmd_active = 0;
        state->cmd_len = 0;
        gtk_label_set_text(GTK_LABEL(state->status_label), "[COMMAND]");
        return TRUE;
    }

    if (state->mode == 0) {
        if (event->keyval == GDK_KEY_i) {
            state->mode = 1;
            gtk_label_set_text(GTK_LABEL(state->status_label), "[INSERT]");
            return TRUE;
        } else if (event->keyval == GDK_KEY_colon) {
            state->cmd_active = 1;
            state->cmd_len = 0;
            state->cmd_buf[0] = '\0';
            char status[128];
            snprintf(status, sizeof(status), "[COMMAND] :%s", state->cmd_buf);
            gtk_label_set_text(GTK_LABEL(state->status_label), status);
            return TRUE;
        } else if (state->cmd_active) {
            if (event->keyval == GDK_KEY_Return) {
                if (strcmp(state->cmd_buf, "w") == 0) {
                    sync_file_buffer(state);
                    save_file(state->filename, &state->fb);
                    gtk_label_set_text(GTK_LABEL(state->status_label), "[Saved]");
                } else if (strcmp(state->cmd_buf, "q") == 0) {
                    gtk_main_quit();
                } else if (strcmp(state->cmd_buf, "wq") == 0) {
                    sync_file_buffer(state);
                    save_file(state->filename, &state->fb);
                    gtk_main_quit();
                }
                state->cmd_active = 0;
                state->cmd_len = 0;
            } else if (event->keyval == GDK_KEY_BackSpace) {
                if (state->cmd_len > 0) {
                    state->cmd_buf[--state->cmd_len] = '\0';
                }
            } else if (state->cmd_len < 99 && event->keyval >= 32 && event->keyval <= 126) {
                state->cmd_buf[state->cmd_len++] = event->keyval;
                state->cmd_buf[state->cmd_len] = '\0';
            }
            if (state->cmd_active) {
                char status[128];
                snprintf(status, sizeof(status), "[COMMAND] :%s", state->cmd_buf);
                gtk_label_set_text(GTK_LABEL(state->status_label), status);
            }
            return TRUE;
        }
    }

    return FALSE;
}

int main(int argc, char *argv[]) {
    EditorState state = {0};

    // 获取文件名
    if (argc != 2) {
        printf("用法：%s <文件名>\n", argv[0]);
        printf("请输入文件路径：");
        if (scanf("%776s", state.filename) != 1) {
            printf("Error: 输入有毛病\n");
            return EXIT_FAILURE;
        }
        while (getchar() != '\n');
    } else {
        strncpy(state.filename, argv[1], 776);
        state.filename[776] = '\0';
    }

    // 加载文件
    load_file(state.filename, &state.fb);

    // 初始化GTK
    gtk_init(&argc, &argv);

    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tao's Edition");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 创建主布局（垂直盒子）
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 上边框（绿色）
    GtkWidget *top_border = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(top_border), "<span foreground=\"green\">--------------------------------------------------------------------------------</span>");
    gtk_box_pack_start(GTK_BOX(vbox), top_border, FALSE, FALSE, 0);

    // 文件名
    char filename_label[800];
    snprintf(filename_label, sizeof(filename_label), "<span> %s </span>", state.filename);
    GtkWidget *filename_widget = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(filename_widget), filename_label);
    gtk_box_pack_start(GTK_BOX(vbox), filename_widget, FALSE, FALSE, 0);

    // 下边框（绿色）
    GtkWidget *bottom_border = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(bottom_border), "<span foreground=\"green\">--------------------------------------------------------------------------------</span>");
    gtk_box_pack_start(GTK_BOX(vbox), bottom_border, FALSE, FALSE, 0);

    // 文本区域（使用 GtkSourceView）
    state.text_buffer = gtk_source_buffer_new(NULL);
    GtkWidget *text_view = gtk_source_view_new_with_buffer(state.text_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    // 启用撤销管理（默认已启用，但可以明确设置）
    gtk_source_buffer_set_max_undo_levels(state.text_buffer, -1); // -1 表示无限制
    update_text_buffer(&state);

    // 滚动窗口
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // 状态栏
    state.status_label = gtk_label_new("[COMMAND]");
    gtk_box_pack_start(GTK_BOX(vbox), state.status_label, FALSE, FALSE, 0);

    // 按键事件
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), &state);

    // 显示窗口
    gtk_widget_show_all(window);
    gtk_main();

    printf("\033[32m");
    printf("[ %s was closed ] \n", state.filename);
    printf("\033[0m");
    return EXIT_SUCCESS;
}

/*printf("\033[32m");
    printf("[ %s is shutdown ] \n", filename);
    printf("\033[0m");*/