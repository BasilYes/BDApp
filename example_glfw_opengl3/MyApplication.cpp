#include "imgui.h"
#include "MySQL.h"

namespace MyApp
{
    struct table
    {
        std::string name;
        std::string sub_name;
        std::vector<MySQL::column> columns;
        std::vector<std::string> joins;
    };

    struct request
    {
        enum {
            norequest,
            search,
            addition,
            remove
        } type;
        int row;
    };

    std::vector<table> tables;
    MySQL::table current_table;
    std::vector<char*> buffers;
    request current_request{request::norequest};

    void process_request()
    {
        if (current_request.type)
        {
            switch (current_request.type)
            {
            case request::search:
                current_table = MySQL::get_search_in_table(current_table.columns, current_table.name, buffers);
                break;
            case request::addition:

                break;
            case request::remove:
                current_table = MySQL::delete_from_table(current_table.columns, current_table, current_request.row);
            default:
                break;
            }
            current_request.type = request::norequest;
        }
    }

    void InitApp()
    {
        MySQL::init_mysql();
        tables.push_back(table
            {
            u8"workers",
            u8"Работники",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_worker", u8"Номер работника", MySQL::INT},
            MySQL::column{0, u8"first_first_name", u8"Имя", MySQL::STR},
            MySQL::column{1, u8"first_last_name", u8"Фамилия", MySQL::STR},
            MySQL::column{2, u8"patronymic", u8"Отчество", MySQL::STR},
            MySQL::column{3, u8"job_title", u8"Должность", MySQL::STR}
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"assembly",
            u8"Сборка",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_assembly", u8"Номер операции", MySQL::INT},
            MySQL::column{1, u8"id_worker", u8"Номер работника", MySQL::INT},
            MySQL::column{2, u8"id_watch_model", u8"Номер модели часов", MySQL::INT},
            MySQL::column{3, u8"id_assembly_stage", u8"Этап сборки", MySQL::INT},
            MySQL::column{3, u8"count", u8"Количество", MySQL::INT},
        },
            std::vector<std::string>{}
            });
    }

    void changeCurrentTable(table& tab)
    {
        current_table = MySQL::get_table(tab.columns, tab.name);
        for (char* buff : buffers)
            delete buff;
        buffers.clear();
        for (int i = 0; i < tab.columns.size(); i++)
            buffers.push_back(new char[100]{""});
    }

    void RenderDBList()
    {
        ImGui::Begin(u8"Таблицы");

        for (table& tab : tables)
        {
            if (ImGui::Button(tab.sub_name.c_str(), ImVec2(-FLT_MIN, 0.0f)))
                changeCurrentTable(tab);
        }

        ImGui::End();
    }

    void RenderData()
    {
        ImGui::Begin(u8"Данные");

        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

        if (current_table.columns.size() > 0)
            if (ImGui::BeginTable("table1", current_table.columns.size()+1, flags))
            {
                for (MySQL::column& column : current_table.columns)
                    ImGui::TableSetupColumn(column.sub_name.c_str(), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"Действие", ImGuiTableColumnFlags_WidthStretch);

                //ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
                //ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                ImGui::TableNextRow();
                {
                    int column;
                    for (column = 0; column < current_table.columns.size(); column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        switch (current_table.columns[column].type)
                        {
                        case MySQL::INT:
                            ImGui::InputTextMultiline(std::to_string(column).c_str(), buffers[column], 64, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 1.5f), ImGuiInputTextFlags_CharsDecimal);
                            break;
                        case MySQL::STR:
                            ImGui::InputTextMultiline(std::to_string(column).c_str(), buffers[column], 64, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 1.5f));
                            break;
                        default:
                            ImGui::Text("%s %d", "Error_type", column);
                            break;
                        }
                    }
                    ImGui::TableSetColumnIndex(column);
                    if (ImGui::Button(u8"Поиск"))
                    {
                        std::cout << "search" << std::endl;
                        current_request.type = request::search;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(u8"Добавить"))
                        current_request.type = request::addition;
                    //ImGui::SameLine();
                    //ImGui::Button(u8"Изменить");
                }

                for (int row = 0; row < current_table.row_count; row++)
                {
                    ImGui::TableNextRow();
                    {
                        int column;
                        for (column = 0; column < current_table.columns.size(); column++)
                        {
                            ImGui::TableSetColumnIndex(column);
                            switch (current_table.columns[column].type)
                            {
                            case MySQL::INT:
                                ImGui::Text("%d", current_table.int_columns[current_table.columns[column].id][row]);
                                break;
                            case MySQL::STR:
                                ImGui::Text("%s", current_table.str_columns[current_table.columns[column].id][row].c_str());
                                break;
                            default:
                                ImGui::Text("%s %d,%d", "Error_type", column, row);
                                break;
                            }
                        }
                        ImGui::TableSetColumnIndex(column);
                        if (ImGui::Button((std::string(u8"Удалить##")+ std::to_string(row)).c_str()))
                        {
                            std::cout << "remove" << std::endl;
                            current_request.type = request::remove;
                            current_request.row = row;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button((std::string(u8"Изменить##") + std::to_string(row)).c_str()))
                            std::cout << "change" << std::endl;
                    }
                }
                ImGui::EndTable();
            }

        ImGui::End();
    }

    void RenderProcedures()
    {
        ImGui::Begin(u8"Запросы");

        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));
        ImGui::Button(u8"Button", ImVec2(-FLT_MIN, 0.0f));

        ImGui::End();
    }

    void RenderUI()
    {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        RenderData();
        RenderProcedures();
        RenderDBList();

        ImGui::End();

        ImGui::ShowDemoWindow();

        process_request();
    }
}
