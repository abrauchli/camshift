//--------------------------------------------------------------------------
//
// File name:      cmdln.h
//
// Abstract:       Templatized command line parser.
//
//--------------------------------------------------------------------------
#ifndef __GLUE_CMDLN_H__
#define __GLUE_CMDLN_H__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <set>

//
// 
//
namespace cmdln
    {

    class help_exception_t
        :   public std::exception
        {
        public:
    
            help_exception_t(const std::string &helpstr)
                {
                help_txt = helpstr;
                }
    
            virtual ~help_exception_t() throw() {}
            virtual const char* what() const throw() { return help_txt.c_str(); }
    
        private:
    
            std::string             help_txt;
        };


    //-----------------------------------------------------------------------------
    //
    // class:       opt_t
    //
    // Abstract:
    //  Base class for all command line option types to derive from.
    // 
    //-----------------------------------------------------------------------------
    class opt_t
        {
        friend class parser_t;
    
        public:
        
            bool opt_match(const std::string &Opt)
                {
                bool    match;
                
                if ( Opt[0] != '-' && !named() )
                    {
                    match = true;
                    }
                else if (Opt == short_name || Opt == long_name)
                    {
                    match = true;
                    }
                else
                    {
                    match = false;
                    }
                    
                return match;
                }
        
            virtual char** parse(char** ArgVal) = 0;
        
        protected:
        
            opt_t(const std::string &ShortName,
                  const std::string &LongName,
                  const std::string &Desc,
                  bool              Valid = false)
                :   short_name(ShortName),
                    long_name(LongName),
                    desc(Desc),
                    valid(Valid)
            {
                short_name = "-" + short_name;
                long_name = "--" + long_name;
            }

        
            opt_t()
                :   short_name(""),
                    long_name(""),
                    desc(""),
                    valid(true)
            {
            }

            
            bool named() const
                {
                return !short_name.empty() || !long_name.empty();
                }
                  
            bool                valid;
            std::string         short_name;
            std::string         long_name;
            std::string         desc;
        };
    
    
    
    //-----------------------------------------------------------------------------
    //
    // class:       opt_val_t
    //
    // Abstract:
    //  Generic template class for all simple value types.  This would mostly be
    // numeric types as strings and bools are specialized below.
    // 
    //-----------------------------------------------------------------------------
    template <typename T>
    class opt_val_t : public opt_t
        {
        public:
        
            opt_val_t(const std::string     &ShortName,
                      const std::string     &LongName,
                      const std::string     &Desc)
                :   opt_t(ShortName, LongName, Desc)
                {
                }
        
            opt_val_t(const std::string     &ShortName,
                      const std::string     &LongName,
                      const std::string     &Desc,
                      const T               &Default)
                :   opt_t(ShortName, LongName, Desc),
                    val(Default)
                {
                }
                
            opt_val_t() {}
        
            virtual char** parse(char **ArgVal)
                {
                if ( named() )
                    {
                    ArgVal++;
                    }
                    
                std::stringstream    ss(*ArgVal);
        
                ss >> val;
                valid = true;
        
                return ArgVal + 1;
                }
        
            const T& value() const
                {
                return val;
                }
        
            operator const T& () const
                {
                return val;
                }
        
        private:
        
            T               val;
            };

    
    //-----------------------------------------------------------------------------
    //
    // class:       opt_val_t<bool>
    //
    // Abstract:
    //  Specialized boolean opt value.  It behaves like a switch and does
    // not have an associated "value" - it's either on or off.
    // 
    //-----------------------------------------------------------------------------
    template <>
    class opt_val_t<bool> : public opt_t
        {
    public:
    
        opt_val_t(const std::string &ShortName,
                  const std::string &LongName,
                  const std::string &Desc,
                  bool              Default = false)
            :   opt_t(ShortName, LongName, Desc, true),
                on(Default)
            {
            }
            
        opt_val_t() {}
    
        operator bool () const
            {
            return on;
            }
            
        bool value() const
            {
            return on;
            }
    
        virtual char** parse(char **ArgVal)
            {
            on = true;
    
            return ArgVal + 1;
            }
    
    private:
    
        bool            on;
    };
        
    template <>
    class opt_val_t<std::string> 
        :   public opt_t
        {
        public:
        
            opt_val_t(const std::string  &ShortName,
                      const std::string  &LongName,
                      const std::string  &Desc,
                      const std::string  &Default = "")
                :   opt_t(ShortName, LongName, Desc),
                    val(Default)
                {
                }
                
            opt_val_t() {}
        
            virtual char** parse(char **ArgVal)
                {
                if ( named() )
                    {
                    ArgVal++;
                    }
        
                val = *ArgVal;
                valid = true;
        
                return ArgVal + 1;
                }
        
            const std::string& value() const
                {
                return val;
                }
        
            operator std::string () const
                {
                return val;
                }

            bool operator== (const opt_val_t &Opt)
                {
                return val == Opt.val;
                }

            bool operator!= (const opt_val_t &Opt)
                {
                return val != Opt.val;
                }

            bool operator== (const std::string &Val)
                {
                return val == Val;
                }

            bool operator!= (const std::string &Val)
                {
                return val != Val;
                }

        private:
        
            std::string         val;
        };
        
    template <typename Ty>
    class opt_list_t : public opt_t
        {
        public:
        
            opt_list_t(const std::string    &ShortName,
                       const std::string    &LongName,
                       const std::string    &Desc)
                :   opt_t(ShortName, LongName, Desc)
            {
            }
        
            virtual char** parse(char **ArgVal)
            {
                opt_val_t<Ty>   vopt;
                char            **argv;

                std::cout << "here" << std::endl;
                
                argv = vopt.parse(ArgVal);
                opt_list.push_back(vopt);
                
                valid = true;
                
                return argv;
            }
                
            int size() const
            {
                return opt_list.size();
            }
                
            const Ty& operator[] (int Index)
            {
                return opt_list[Index];
            }
                
            const Ty& value(int Index)
                {
                return opt_list[Index];
                }
                
        private:
            
            std::vector< opt_val_t<Ty> >            opt_list;
        };
        
    template <typename Ty>
    class opt_set_t
        :   public opt_val_t<Ty>
        {
        public:
            opt_set_t(const std::string     &ShortName,
                      const std::string     &LongName,
                      const std::string     &Desc,
                      const std::set<Ty>    &OptSet)
                :   opt_val_t<Ty>(ShortName, LongName, Desc),
                    opt_enum(OptSet)
                {
                common_init();
                }
        
        
            opt_set_t(const std::string     &ShortName,
                      const std::string     &LongName,
                      const std::string     &Desc,
                      const std::set<Ty>    &OptSet,
                      const Ty              &Default)
                :   opt_val_t<Ty>(ShortName, LongName, Desc, Default),
                    opt_enum(OptSet)
                {
                common_init();
                }
                
            virtual char** parse(char **ArgVal)
                {
                char                                **argv;
                typename std::set<Ty>::iterator     it;
                
                argv = opt_val_t<Ty>::parse(ArgVal);
                
                it = opt_enum.find( static_cast<const Ty&>(*this) );
                
                if ( it == opt_enum.end() )
                    {
                    std::stringstream   e_strm;

                    e_strm << "Invalid command line option \"" << *ArgVal << "\".";
                    throw std::invalid_argument( e_strm.str() );
                    }
                
                return argv;
                }
        
        private:
        
            std::set<Ty>                    opt_enum;
        
            void common_init()
                {
                typename std::set<Ty>::iterator     it,
                                                    next_to_last;
        
                next_to_last = opt_enum.end();
                next_to_last--;
        
                opt_val_t<Ty>::desc += " (";
                for ( it = opt_enum.begin(); it != next_to_last; it++ )
                    {
                    opt_val_t<Ty>::desc += *it + ", ";
                    }
        
                opt_val_t<Ty>::desc += *it + ").";
                }
        };
        
    
    class parser_t
        {
        public:
            parser_t()
                :   help_info("h", "help", "Show help and usage information.")
            {
                add(help_info);
            }
        
            parser_t(const std::string &Name, const std::string &Description="")
                :   help_info("h", "help", "Show help and usage information.", false),
                    name(Name),
                    desc(Description)
            {
                add(help_info);
            }
        
            void parse(int ArgCnt, char **ArgVal)
            {
                char    **end_arg = &ArgVal[ArgCnt];
            
                ArgVal += 1;
            
                while (ArgVal < end_arg)
                {
                    int opt_i = 0;
            
                    while ( opt_i < cl_opts.size() && !cl_opts[opt_i]->opt_match(*ArgVal) )
                    {
                        opt_i += 1;
                    }
            
                    if ( opt_i < cl_opts.size() )
                    {
                        ArgVal = cl_opts[opt_i]->parse(ArgVal);
                    }
                    else if (*ArgVal[0] == '-')
                    {
                        std::stringstream    e_strm;

                        e_strm << "Unknown command line option \"" << *ArgVal << "\".";
                        throw std::invalid_argument( e_strm.str() );
                    }
                    else
                    {
                        std::stringstream       e_strm;

                        e_strm << "Invalid option syntax \"" << *ArgVal << "\".";
                        throw std::invalid_argument( e_strm.str() );
                    }
                }
            
                if (help_info)
                {
                    throw help_exception_t( help() );
                }
            }



            void add(opt_t &Opt)
            {
                cl_opts.push_back(&Opt);
            }

            std::string help()
            {
                std::vector<opt_t*>::iterator   opt_it;
                std::stringstream               help_str;
            
                help_str << std::endl << name << ":" << std::endl << std::endl;
            
                for ( opt_it = cl_opts.begin(); opt_it != cl_opts.end(); opt_it += 1 )
                {
                    opt_t           *p_opt = *opt_it;
                    std::string     usage;
            
                    usage = p_opt->short_name;
                    if (usage != "")
                    {
                        usage += ", ";
                    }
                    usage += p_opt->long_name + "\t\t" + p_opt->desc;
            
                    help_str << usage << std::endl;
                }
            
                return help_str.str();
            }

        
        private:
        
            opt_val_t<bool>                 help_info;
            std::string                     name;
            std::string                     desc;
            std::vector<opt_t*>             cl_opts;
        };
    
    

    }; // 'glu' namespace

#endif



