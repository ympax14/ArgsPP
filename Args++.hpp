#ifndef ARGSPP_HPP
#define ARGSPP_HPP

#include <iostream>
#include <stdexcept>
#include <map>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <iomanip>

class ArgsPP {
    public:
        class Arg {
            private:
                std::string value;
                std::string description;
                std::string defaultValue;
                bool required;
                std::unordered_set<std::string> allowedValues;

            public:
                Arg(const std::string &_value, const std::string &_description, const std::string &_defaultValue, bool _required) : value(_value), description(_description), defaultValue(_defaultValue), required(_required), allowedValues(std::unordered_set<std::string>()) {};
                ~Arg() = default;

                Arg& operator=(const Arg &a) = default;

                inline void setValue(std::string value) {
                    if (!this->allowedValues.empty() && this->allowedValues.find(value) == this->allowedValues.end())
                        throw std::invalid_argument("La nouvelle valeur '" + value + "' n'est pas présente dans les valeurs autorisées.");

                    this->value = value;
                };

                inline const std::string& getValue() const {
                    return this->value.empty() ? this->defaultValue : this->value;
                }

                inline const std::string& getDescription() const {
                    return this->description;
                }

                inline const bool& isRequired() const {
                    return this->required;
                }

                inline const std::unordered_set<std::string>& getAllowedValues() const {
                    return this->allowedValues;
                }

                inline void addAllowedValue(const std::string& value) {
                    this->allowedValues.insert(value);
                }

                bool toBoolean() const {
                    std::string valueCopy = std::string(this->getValue());
                    std::transform(valueCopy.begin(), valueCopy.end(), valueCopy.begin(), ::tolower);
                    std::istringstream is(valueCopy);
                    
                    bool b = false;
                    is >> std::boolalpha >> b;

                    return b;
                };

                inline int toInt() const {
                    return std::stoi(this->getValue());
                };

                inline double toDouble() const {
                    return std::stod(this->getValue());
                };

                inline float toFloat() const {
                    return std::stof(this->getValue());
                };

                inline const char* toCChar() const {
                    return this->getValue().c_str();
                };
        };

    private:
        std::string ARGS_PREFIX;
        std::map<std::string, Arg> arguments;

        inline bool isArgument(const char str[]) {
            return std::strncmp(ARGS_PREFIX.c_str(), str, ARGS_PREFIX.length()) == 0;
        }

        void checkArgumentsIntegrity() {
            for (auto& pair : this->arguments) {
                Arg& arg = pair.second;

                if (arg.isRequired() && arg.getValue().empty())
                    throw std::runtime_error("L'argument '" + pair.first + "' est requis mais aucune valeur n'a été spécifiée.");
            }
        }

        int getRequiredArgumentsCount() {
            int count = 0;

            for (auto& pair : this->arguments)
                if (pair.second.isRequired())
                    count++;

            return count;
        }

    public:
        ArgsPP() : ARGS_PREFIX("--") {};
        ArgsPP(const ArgsPP&) = delete;
        ~ArgsPP() = default;

        const Arg* operator[](const std::string& key) const {
            auto it = this->arguments.find(key);

            if (it != this->arguments.end()) {
                return &it->second;
            }

            return nullptr; // Retourne vide si la clé n'existe pas
        }
        
        inline void setArgumentsPrefix(const char prefix[]) {
            this->ARGS_PREFIX = std::string(prefix);
        }

        inline void setArgumentsPrefix(const std::string &prefix) {
            this->ARGS_PREFIX = prefix;
        }

        Arg& registerArgument(const std::string &key, const std::string& description, bool required = false, const std::string &defaultValue = "") {
            if (!required && defaultValue.empty())
                throw std::invalid_argument("Un argument non-requis nécessite une valeur par défaut.");

            if (this->arguments.find(key) != this->arguments.end())
                throw std::invalid_argument("L'enregistrement de l'argument '" + key + "' n'est pas possible puisqu'un argument avec la même clé est déjà enregistré.");
            
            auto result = arguments.emplace(key, Arg("", description, defaultValue, required));
            return result.first->second;
        }

        void printHelp() const {
            std::cout << std::endl << "Options disponibles :" << std::endl;

            if (arguments.empty()) {
                std::cout << "  Aucun argument n'est configuré." << std::endl;
                return;
            }

            size_t maxKeyLength = 0;
            for (const auto& pair : this->arguments)
                if (pair.first.length() > maxKeyLength)
                    maxKeyLength = pair.first.length();

            for (const auto& pair : this->arguments) {
                const std::string& key = pair.first;
                const Arg& arg = pair.second;

                if (arg.getDescription().empty())
                    continue;

                std::cout << "  " << ARGS_PREFIX;
                std::cout << std::left << std::setw(maxKeyLength + 4) << key;

                std::cout << arg.getDescription();

                if (arg.isRequired()) {
                    std::cout << " [Requis]";
                } else {
                    std::cout << " [Défaut : " << (arg.getValue().empty() ? "\"\"" : arg.getValue()) << "]";
                }
                std::cout << std::endl;

                const auto& allowed = arg.getAllowedValues();
                if (!allowed.empty()) {
                    std::string padding(ARGS_PREFIX.length() + maxKeyLength + 6, ' ');
                    std::cout << padding << "(Valeurs autorisées : ";
                    
                    bool first = true;
                    for (const auto& val : allowed) {
                        if (!first) std::cout << ", ";
                        std::cout << val;
                        first = false;
                    }
                    std::cout << ")" << std::endl;
                }
            }
            std::cout << std::endl;
        }

        void parseArgs(int count, char* cmdArgs[]) {
            // Remove app's calling argument
            count -= 1;
            if (count == 0) {
                this->printHelp();
                exit(EXIT_SUCCESS);
                return;
            }

            cmdArgs += 1;

            for (int i = 0; i < count; i++) {
                if (this->isArgument(cmdArgs[i])) {
                    std::string key(cmdArgs[i] + ARGS_PREFIX.length());
                    if (key == "help" || key == "h") {
                        this->printHelp();
                        exit(EXIT_SUCCESS);
                        return;
                    }
                }
            }

            int reqArgsCount = this->getRequiredArgumentsCount();
            if (count < reqArgsCount)
                throw std::runtime_error("Nombre insuffisant d'arguments. " + std::to_string(reqArgsCount) + " argument(s) requis, mais seulement " + std::to_string(count) + " fourni(s).");

            for (int i = 0; i < count; i++) {
                if (!this->isArgument(cmdArgs[i])) continue;

                std::string key(cmdArgs[i]+ARGS_PREFIX.length());
                std::string value = "";

                if (i + 1 < count && !this->isArgument(cmdArgs[i+1])) {
                    value = cmdArgs[i+1];
                    i++; // On "consomme" la valeur pour ne pas la relire à la prochaine itération
                } else {
                    // C'est un flag booléen sans valeur explicite (ex: --verbose)
                    value = "true"; 
                }

                std::map<std::string, Arg>::iterator it = this->arguments.find(key);
                if (it != this->arguments.end()) {
                    it->second.setValue(value);
                } else arguments.emplace(key, Arg(value, "", "", false));
            }

            this->checkArgumentsIntegrity();
        }
};

#endif