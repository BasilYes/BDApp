#include "imgui.h"
#include "MySQL.h"
#include <algorithm>
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
            remove,
            changing,
            change,
            refresh_table,
            select,
            noselect
        } type;
        int row;
    };

    struct procedure
    {
        std::string name;
        std::string comand;
        std::vector<MySQL::column> columns;
    };

    bool show_interact = true;
    bool show_request = false;
    int sorted_column = 0;
    bool sort_derection;
    std::vector<table> tables;
    std::vector<procedure> procedures;
    MySQL::table current_table;
    table* current_table_ref;
    std::vector<char*> buffers;
    std::vector<int> current_keys;
    request current_request{ request::norequest };

    void refresh_buffers()
    {
        table& tab = *current_table_ref;
        for (char* buff : buffers)
            delete[] buff;
        buffers.resize(tab.columns.size());
        current_keys.resize(tab.columns.size());
        for (int i = 0; i < tab.columns.size(); i++)
        {
            buffers[i] = new char[100] {""};
            current_keys[i] = -1;
        }
    }

    std::vector<std::string> comand_parts{
    "SELECT",
    "FROM",
    "WHERE",
    "GROUP BY",
    "ORDER BY",
    };
    char rec_text[1000] {""};
    void process_request()
    {
        if (current_request.type)
        {
            std::vector<int> keys;
            keys.resize(current_keys.size());
            for (int i = 0; i < keys.size(); i++)
            {
                keys[i] = 0;
                if (current_keys[i] != -1)
                    keys[i] = current_table.int_content_columns[current_table.columns[i].id][current_keys[i]];
            }

            switch (current_request.type)
            {
            case request::search:
                current_table = MySQL::get_search_in_table(current_table.columns, current_table_ref->joins, current_table.name, buffers, keys, current_keys, sorted_column, sort_derection);
                break;
            case request::addition:
                current_table = MySQL::add_to_table(current_table.columns, current_table_ref->joins, current_table.name, buffers, keys, current_keys, sorted_column, sort_derection);
                refresh_buffers();
                break;
            case request::remove:
                current_table = MySQL::delete_from_table(current_table.columns, current_table_ref->joins, current_table, current_request.row, sorted_column, sort_derection);
            case request::change:
                current_table = MySQL::edit_row(current_table.columns, current_table_ref->joins, current_table.name, buffers, keys, current_keys, current_table, current_request.row, sorted_column, sort_derection);
                refresh_buffers();
            case request::refresh_table:
                current_table = MySQL::resort_table(current_table.columns, current_table.name, sorted_column, sort_derection);
                break;
            case request::select:
            {
                std::string a;
                for (int i = 0; i < 5; i++)
                {
                    if (std::string(buffers[i]) != "")
                    {
                        a += comand_parts[i] + " " + buffers[i] + " ";
                    }
                }
                current_table = MySQL::custom_mysql_request(a);
                show_request = false;
                show_interact = false;
            }
            break;
            case request::noselect:
            {
                current_table = MySQL::custom_mysql_proc(rec_text);
                show_request = false;
                show_interact = false;
            }
            break;
            default:
                break;
            }
            if (current_request.type != request::changing)
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
            MySQL::column{0, u8"id_worker", u8"Номер работника", u8"id_worker", u8"workers", MySQL::INT},
            MySQL::column{0, u8"first_first_name", u8"Имя", u8"first_first_name", u8"workers", MySQL::STR},
            MySQL::column{1, u8"first_last_name", u8"Фамилия", u8"first_last_name", u8"workers", MySQL::STR},
            MySQL::column{2, u8"patronymic", u8"Отчество", u8"patronymic", u8"workers", MySQL::STR},
            MySQL::column{3, u8"job_title", u8"Должность", u8"job_title", u8"workers", MySQL::STR}
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"assembly",
            u8"Сборка",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_assembly", u8"Номер операции", u8"id_assembly", u8"assembly", MySQL::INT},
            MySQL::column{0, u8"first_last_name", u8"Работник", u8"id_worker", u8"workers", MySQL::KEY},
            MySQL::column{1, u8"watch_model_name", u8"Модель часов", u8"id_watch_model", u8"watch_model", MySQL::KEY},
            MySQL::column{2, u8"assembly_stage_name", u8"Этап сборки", u8"id_assembly_stage", u8"assembly_stages", MySQL::KEY},
            MySQL::column{1, u8"count", u8"Количество", u8"count", u8"assembly", MySQL::INT},
        },
            std::vector<std::string>{"watch_model USING(id_watch_model)", "assembly_stages USING(id_assembly_stage)", "workers USING(id_worker)"}
            });
        tables.push_back(table
            {
            u8"storages",
            u8"Хранилище",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_storage", u8"Номер ячейки", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"detail_name", u8"Деталь", u8"id_detail", u8"detailes", MySQL::KEY},
            MySQL::column{1, u8"count", u8"Количество", u8"", u8"", MySQL::INT},
        },
            std::vector<std::string>{"detailes USING(id_detail)"}
            });
        tables.push_back(table
            {
            u8"detailes",
            u8"Детали",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_detail", u8"Номер детали", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"detail_name", u8"Наименование", u8"", u8"", MySQL::STR},
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"watch_model",
            u8"Модель часов",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_watch_model", u8"Номер модели", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"mechanism_type_name", u8"Тип механизма", u8"id_mechanism_type", u8"mechanism_type", MySQL::KEY},
            MySQL::column{0, u8"watch_model_name", u8"Название модели", u8"", u8"", MySQL::STR},
        },
            std::vector<std::string>{"mechanism_type USING(id_mechanism_type)"}
            });
        tables.push_back(table
            {
            u8"mechanism_type",
            u8"Тип механизма",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_mechanism_type", u8"Номер типа", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"mechanism_type_name", u8"Название типа", u8"", u8"", MySQL::STR},
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"remains",
            u8"Остатки",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_remains", u8"Номер остатков", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"workplace_name", u8"Рабочее место", u8"id_workplace", u8"workplaces", MySQL::KEY},
            MySQL::column{1, u8"detail_name", u8"Деталь", u8"id_detail", u8"detailes", MySQL::KEY},
            MySQL::column{1, u8"count", u8"Количество", u8"", u8"", MySQL::INT},
        },
            std::vector<std::string>{"workplaces USING(id_workplace)","detailes USING(id_detail)"}
            });
        tables.push_back(table
            {
            u8"workplaces",
            u8"Рабочие места",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_workplace", u8"Номер рабочего места", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"workplace_name", u8"Наименование", u8"", u8"", MySQL::STR},
            MySQL::column{0, u8"first_last_name", u8"Работник", u8"id_worker", u8"workers", MySQL::KEY},
        },
            std::vector<std::string>{"workers USING(id_worker)"}
            });
        tables.push_back(table
            {
            u8"finished_products",
            u8"Произведенная продукция",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_finished_products", u8"Номер партии", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"watch_model_name", u8"Модель часов", u8"id_watch_model", u8"watch_model", MySQL::KEY},
            MySQL::column{1, u8"count", u8"Количество", u8"", u8"", MySQL::INT},
        },
            std::vector<std::string>{"watch_model USING(id_watch_model)"}
            });
        tables.push_back(table
            {
            u8"assembly_stages",
            u8"Стадии сборки",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_assembly_stage", u8"Номер стадии", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"assembly_stage_name", u8"Название этапа", u8"", u8"", MySQL::STR},
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"defect_type",
            u8"Типы дефектов",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_defect_type", u8"Номер типа", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"defect_type_name", u8"Название типа", u8"", u8"", MySQL::STR},
        },
            std::vector<std::string>{}
            });
        tables.push_back(table
            {
            u8"defect",
            u8"Дефектная продукция",
            std::vector<MySQL::column>
        {
            MySQL::column{0, u8"id_defect", u8"Номер записи", u8"", u8"", MySQL::INT},
            MySQL::column{0, u8"watch_model_name", u8"Модель часов", u8"id_watch_model", u8"watch_model", MySQL::KEY},
            MySQL::column{1, u8"defect_type_name", u8"Тип дефекта", u8"id_defect_type", u8"defect_type", MySQL::KEY},
            MySQL::column{1, u8"count", u8"Количество", u8"", u8"", MySQL::INT},
        },
            std::vector<std::string>{"defect_type USING(id_defect_type)", "watch_model USING(id_watch_model)"}
            });




        procedures.push_back(procedure
            {
                std::string(u8"Лучший работник"),
                std::string(u8"SELECT best_worker_name() AS name"),
                std::vector<MySQL::column>
                {
            MySQL::column{0, u8"name", u8"Имя", u8"", u8"", MySQL::STR},
                }
            });
        procedures.push_back(procedure
            {
                std::string(u8"Количество дефектных"),
                std::string(u8"SELECT check_defect_count() AS count"),
                std::vector<MySQL::column>
                {
            MySQL::column{0, u8"count", u8"Количество", u8"", u8"", MySQL::INT},
                }
            });
        procedures.push_back(procedure
            {
                std::string(u8"Популярнейшая модель"),
                std::string(u8"SELECT watch_model_name FROM watch_model WHERE id_watch_model = popularlest_model()"),
                std::vector<MySQL::column>
                {
            MySQL::column{0, u8"watch_model_name", u8"Модель часов", u8"", u8"", MySQL::STR},
                }
            });
    }

    void changeCurrentTable(table& tab)
    {
        current_table = MySQL::get_table(tab.columns, tab.joins, tab.name, 0, 0);
        sorted_column = 0;
        sort_derection = 0;
        current_table_ref = &tab;

        show_interact = true;
        show_request = false;

        refresh_buffers();
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

        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | (show_interact ? ImGuiTableFlags_Sortable : 0);

        if (show_request)
        {

            ImGui::Text("SELECT");
            ImGui::SameLine();
            ImGui::InputTextMultiline("##0_tt", buffers[0], 99, ImVec2(200.0f, ImGui::GetTextLineHeight() * 5.0f));
            ImGui::SameLine();
            ImGui::Text("FROM");
            ImGui::SameLine();
            ImGui::InputTextMultiline("##1_tt", buffers[1], 99, ImVec2(200.0f, ImGui::GetTextLineHeight() * 5.0f));
            ImGui::SameLine();
            ImGui::Text("WHERE");
            ImGui::SameLine();
            ImGui::InputTextMultiline("##2_tt", buffers[2], 99, ImVec2(200.0f, ImGui::GetTextLineHeight() * 5.0f));
            ImGui::SameLine();
            ImGui::Text("GROUP BY");
            ImGui::SameLine();
            ImGui::InputTextMultiline("##3_tt", buffers[3], 99, ImVec2(200.0f, ImGui::GetTextLineHeight() * 5.0f));
            ImGui::SameLine();
            ImGui::Text("ORDER BY");
            ImGui::SameLine();
            ImGui::InputTextMultiline("##4_tt", buffers[4], 99, ImVec2(200.0f, ImGui::GetTextLineHeight() * 5.0f));
            if (ImGui::Button(u8"Select", ImVec2(-FLT_MIN, 0.0f)))
            {
                /*std::cout << "select ";
                std::string a;
                for (int i = 0; i < 5; i++)
                {
                    if (std::string(buffers[i]) != "")
                    {
                        a += comand_parts[i] + " " + buffers[i] + " ";
                    }
                }
                current_table = MySQL::custom_mysql_request(a);
                show_request = false;
                show_interact = false;*/
                current_request.type = request::select;
            }
            ImGui::InputTextMultiline("##5_tt", rec_text, 999, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5.0f));
            if (ImGui::Button(u8"Все кроме SELECT", ImVec2(-FLT_MIN, 0.0f)))
            {
                /*current_table = MySQL::custom_mysql_proc(rec_text);
                show_request = false;
                show_interact = false;*/
                current_request.type = request::noselect;
            }
        }
        else
            if (current_table.columns.size() > 0)
                if (ImGui::BeginTable("table1", current_table.columns.size() + (show_interact ? 1 : 0), flags))
                {
                    for (int column = 0; column < current_table.columns.size(); column++)
                        ImGui::TableSetupColumn(current_table.columns[column].printed_name.c_str(), ImGuiTableColumnFlags_WidthFixed | (show_interact ? ImGuiTableColumnFlags_DefaultSort : 0), 0.0f, column);
                    if (show_interact)
                        ImGui::TableSetupColumn(u8"Действие", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort);

                    if (show_interact)
                        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                            if (sorts_specs->SpecsDirty)
                            {
                                if (sorts_specs->Specs->ColumnUserID != sorted_column || (sorts_specs->Specs->SortDirection != ImGuiSortDirection_Ascending) != sort_derection)
                                {
                                    sorted_column = sorts_specs->Specs->ColumnUserID;
                                    sort_derection = sorts_specs->Specs->SortDirection != ImGuiSortDirection_Ascending;

                                    current_request.type = request::refresh_table;
                                }
                                sorts_specs->SpecsDirty = false;
                            }

                    //ImGui::TableSetupColumn("BBB", ImGuiTableColumnFlags_WidthFixed);
                    //ImGui::TableSetupColumn("CCC", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();
                    if (show_interact)
                    {
                        ImGui::TableNextRow();
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
                            case MySQL::KEY:
                            {
                                int id = current_table.columns[column].id;
                                ImGui::PushItemWidth(-1);
                                if (ImGui::BeginCombo((std::string("##combo") + std::to_string(column)).c_str(),
                                    current_keys[column] >= 0 ? current_table.str_content_columns[id][current_keys[column]].c_str() : " ",
                                    ImGuiComboFlags_NoArrowButton))
                                {
                                    {
                                        const bool is_selected = (current_keys[column] == -1);
                                        if (ImGui::Selectable(" ", is_selected))
                                            current_keys[column] = -1;

                                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                        if (is_selected)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    for (int n = 0; n < current_table.str_content_columns[id].size(); n++)
                                    {
                                        const bool is_selected = (current_keys[column] == n);
                                        if (ImGui::Selectable(current_table.str_content_columns[id][n].c_str(), is_selected))
                                            current_keys[column] = n;

                                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                        if (is_selected)
                                            ImGui::SetItemDefaultFocus();
                                    }
                                    ImGui::EndCombo();
                                }
                                ImGui::PopItemWidth();
                            }
                            break;
                            default:
                                ImGui::Text("%s %d", "Error_type", column);
                                break;
                            }
                        }
                        ImGui::TableSetColumnIndex(column);
                        if (current_request.type != request::changing)
                        {
                            if (ImGui::Button(u8"Поиск"))
                                current_request.type = request::search;
                            ImGui::SameLine();
                            if (ImGui::Button(u8"Добавить"))
                                current_request.type = request::addition;
                        }
                        else
                        {
                            if (ImGui::Button(u8"Сохранить"))
                                current_request.type = request::change;
                            ImGui::SameLine();
                            if (ImGui::Button(u8"Отмена"))
                                current_request.type = request::norequest;

                        }

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
                                {
                                    auto a = current_table.str_columns[current_table.columns[column].id][row].c_str();
                                    ImGui::Text("%s", a);
                                }
                                    break;
                                case MySQL::KEY:
                                    ImGui::Text("%s", current_table.str_key_columns[current_table.columns[column].id][row].c_str());
                                    break;
                                default:
                                    ImGui::Text("%s %d,%d", "Error_type", column, row);
                                    break;
                                }
                            }
                            if (show_interact)
                            {
                                ImGui::TableSetColumnIndex(column);
                                if (ImGui::Button((std::string(u8"Удалить##") + std::to_string(row)).c_str()))
                                {
                                    current_request.type = request::remove;
                                    current_request.row = row;
                                }
                                ImGui::SameLine();
                                if (ImGui::Button((std::string(u8"Изменить##") + std::to_string(row)).c_str()))
                                {
                                    current_request.type = request::changing;
                                    current_request.row = row;
                                    for (int i = 0; i < current_table.columns.size(); i++)
                                    {

                                        switch (current_table.columns[i].type)
                                        {
                                        case MySQL::INT:
                                        {
                                            int counter = 0;
                                            for (char c : std::to_string(current_table.int_columns[current_table.columns[i].id][row]))
                                            {
                                                buffers[i][counter] = c;
                                                counter++;
                                            }
                                            buffers[i][counter] = '\0';
                                        }
                                        break;
                                        case MySQL::STR:
                                        {
                                            int counter = 0;
                                            for (char c : current_table.str_columns[current_table.columns[i].id][row])
                                            {
                                                buffers[i][counter] = c;
                                                counter++;
                                            }
                                            buffers[i][counter] = '\0';
                                        }
                                        break;
                                        case MySQL::KEY:
                                        {
                                            std::vector<int>& vec = current_table.int_content_columns[current_table.columns[i].id];
                                            current_keys[i] = std::distance(vec.begin(), std::find(vec.begin(), vec.end(), current_table.int_key_columns[current_table.columns[i].id][row]));
                                        }
                                        break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    ImGui::EndTable();
                }

        ImGui::End();
    }

    void RenderProcedures()
    {
        ImGui::Begin(u8"Запросы");
        if (ImGui::Button(u8"Специальный запрос", ImVec2(-FLT_MIN, 0.0f)))
        {
            show_request = true;
            for (char* buff : buffers)
                delete[] buff;
            buffers.resize(5);
            for (int i = 0; i < 5; i++)
            {
                buffers[i] = new char[100] {""};
            }
        }

        for (procedure& proc : procedures)
        {
            if (ImGui::Button(proc.name.c_str(), ImVec2(-FLT_MIN, 0.0f)))
            {
                current_table = MySQL::mysql_request(proc.columns, proc.comand, "NONE");
                show_interact = false;
                show_request = false;
            }
        }

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

        //ImGui::ShowDemoWindow();

        process_request();
    }
}
