#ifndef __I_PROPERTIES_H__
#define __I_PROPERTIES_H__

//! \brief Properties Interface
class IProperties
{
    public:
        virtual ~IProperties() {};

        /** \returns true if the element name exists. */
        virtual bool hasValue(const std::string& name) const = 0;

        /** \returns the value of element name if it exists - or an empty std::string otherwise. */
        virtual std::string getValue(const std::string& name) const = 0;

        /** \brief sets value of property */
        virtual void setValue(const std::string& name, const std::string& value) = 0;
    
        /** \brief deletes property */
        virtual void deleteProperty(const std::string& name) = 0;
};

#endif
