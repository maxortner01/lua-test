#pragma once

#include "Lua.hpp"

#include <unordered_map>
#include <memory>

namespace S2D::Lua
{
    struct Table
    {
        /**
         * @brief Represents a value in the table stored in memory with a type.
         */
        struct Data
        {
            std::shared_ptr<void> data;
            int type;

            /**
             * @brief Construct a data entry from a value
             * @tparam T    Type of the data entry
             * @param value Value of the entry
             * @return Data The entry
             */
            template<typename T>
            static Data fromValue(const T& value);

            /**
             * @brief Allocate room and copy the memory of a given value
             * @tparam T    Type of the entry
             * @param value Value of the entry
             * @return std::shared_ptr<void> Pointer to the allocated entry
             */
            template<typename T>
            static std::shared_ptr<void> emplace(const T& value);
        };

        using Map = std::unordered_map<std::string, Data>;

        Table() = default;
        Table(const Map& map);

        /**
         * @brief Constructs a table from the current Lua stack
         * 
         * This assumes that there is a table already at index -1 in the Lua
         * stack. Once executed, it pops the table off the stack completely.
         * 
         * @param L 
         */
        Table(lua_State* L);
        ~Table() = default;

        void fromTable(const Table& table);

        void superimpose(const Table& table);
        void superimpose(const Map& map);

        template<typename T>
        T& get(const std::string& name);

        template<typename T>
        const T& get(const std::string& name) const;

        template<typename T>
        void set(const std::string& name, const T& value);

        const Map& getMap() const;

        void toStack(lua_State* L) const;
    
    private:
        Map dictionary;
    };
}