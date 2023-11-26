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
        Table(State L);
        ~Table() = default;

        template<typename T>
        void each(std::function<void(uint32_t, T&)> lambda);

        template<typename T>
        void each(std::function<void(uint32_t, const T&)> lambda) const;

        template<typename T>
        void try_get(const std::string& name, std::function<void(T&)> lambda);

        template<typename T>
        void try_get(const std::string& name, std::function<void(const T&)> lambda) const;

        /**
         * @brief Make the entries equivalent to another table
         * @param table Table to get the entries from
         */
        void fromTable(const Table& table);

        /**
         * @brief Copy the entries from a table into this table
         * @param table Table to get the entries from
         */
        void superimpose(const Table& table);

        /**
         * @brief Copy the entries from a map into this table
         * @param map Map to get the entries from
         */
        void superimpose(const Map& map);

        bool hasValue(const std::string& name) const;

        /**
         * @brief Get a reference to a value in the table
         * @tparam T   Type of the value (with Lua:: prefix)
         * @param name The value's key
         * @return T&  The reference to the value
         */
        template<typename T>
        T& get(const std::string& name);

        /**
         * @brief Get a constant reference to a value in the table
         * @tparam T   Type of the value (with Lua:: prefix)
         * @param name The value's key
         * @return const T& The constant reference to the value
         */
        template<typename T>
        const T& get(const std::string& name) const;

        /**
         * @brief Set the value associated with a key
         * @tparam T The type of the value (with Lua:: prefix)
         * @param name  The key
         * @param value The value
         */
        template<typename T>
        void set(const std::string& name, const T& value);

        /**
         * @brief Get the raw mapping of this table
         * @return const Map& The map in this table
         */
        const Map& getMap() const;

        /**
         * @brief Dump all the map information onto the given Lua stack
         * @param L The Lua state to dump the table onto
         */
        void toStack(State L) const;
    
    private:
        Map dictionary;
    };
}