/**
 * IotWebConfParameter.cpp -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <IotWebConfParameter.h>

namespace iotwebconf
{

ParameterGroup::ParameterGroup(
  const char* id, const char* label) :
  ConfigItem(id)
{
  this->label = label;
}

void ParameterGroup::addItem(ConfigItem* configItem)
{
  if (configItem->_parentItem != NULL)
  {
    return; // Item must not be added two times.
  }
  if (this->_firstItem == NULL)
  {
    this->_firstItem = configItem;
    return;
  }
  ConfigItem* current = this->_firstItem;
  while (current->_nextItem != NULL)
  {
    current = current->_nextItem;
  }
  current->_nextItem = configItem;
  configItem->_parentItem = this;
}

int ParameterGroup::getStorageSize()
{
  int size = 0;
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    size += current->getStorageSize();
    current = current->_nextItem;
  }
  return size;
}
void ParameterGroup::applyDefaultValue()
{
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->applyDefaultValue();
    current = current->_nextItem;
  }
}

void ParameterGroup::storeValue(
  std::function<void(SerializationData* serializationData)> doStore)
{
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->storeValue(doStore);
    current = current->_nextItem;
  }
}
void ParameterGroup::loadValue(
  std::function<void(SerializationData* serializationData)> doLoad)
{
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->loadValue(doLoad);
    current = current->_nextItem;
  }
}

void ParameterGroup::renderHtml(
  boolean dataArrived, WebRequestWrapper* webRequestWrapper)
{
    if (this->label != NULL)
    {
      String content = "<fieldset id='";
      content += this->getId();
      content = "'>";
      if (this->label[0] != '\0')
      {
        content += "<legend>";
        content += this->label;
        content += "</legend>";
      }
      webRequestWrapper->sendContent(content);
    }
    ConfigItem* current = this->_firstItem;
    while (current != NULL)
    {
      if (current->visible)
      {
        current->renderHtml(dataArrived, webRequestWrapper);
      }
      current = current->_nextItem;
    }
    if (this->label != NULL)
    {
      webRequestWrapper->sendContent("</fieldset>");
    }
}
void ParameterGroup::update(WebRequestWrapper* webRequestWrapper)
{
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->update(webRequestWrapper);
    current = current->_nextItem;
  }
}
void ParameterGroup::clearErrorMessage()
{
  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    current->clearErrorMessage();
    current = current->_nextItem;
  }
}
void ParameterGroup::debugTo(Stream* out)
{
  out->print('[');
  out->print(this->getId());
  out->println(']');

  // -- Here is some overcomplicated logic to have nice debug ouput.
  boolean ownItem = false;
  boolean lastItem = false;
  PrefixStreamWrapper stream =
    PrefixStreamWrapper(
      out,
      [&](Stream* out1)
    {
      if (ownItem)
      {
        ownItem = false;
        return (size_t)0;
      }
      if (lastItem)
      {
        return out1->print("    ");
      }
      else
      {
        return out1->print("|   ");
      }
    });

  ConfigItem* current = this->_firstItem;
  while (current != NULL)
  {
    if (current->_nextItem == NULL)
    {
      out->print("\\-- ");
    }
    else
    {
      out->print("|-- ");
    }
    ownItem = true;
    lastItem = (current->_nextItem == NULL);
    current->debugTo(&stream);
    current = current->_nextItem;
  }
}

///////////////////////////////////////////////////////////////////////////////

Parameter::Parameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue) :
  ConfigItem(id)
{
  this->label = label;
  this->valueBuffer = valueBuffer;
  this->_length = length;
  this->defaultValue = defaultValue;

  this->errorMessage = NULL;
}
int Parameter::getStorageSize()
{
  return this->_length;
}
void Parameter::applyDefaultValue()
{
  if (defaultValue != NULL)
  {
    strncpy(this->valueBuffer, this->defaultValue, this->getLength());
  }
  else
  {
    this->valueBuffer[0] = '\0';
  }
}
void Parameter::storeValue(
  std::function<void(SerializationData* serializationData)> doStore)
{
  SerializationData serializationData;
  serializationData.length = this->_length;
  serializationData.data = (byte*)this->valueBuffer;
  doStore(&serializationData);
}
void Parameter::loadValue(
  std::function<void(SerializationData* serializationData)> doLoad)
{
  SerializationData serializationData;
  serializationData.length = this->_length;
  serializationData.data = (byte*)this->valueBuffer;
  doLoad(&serializationData);
}
void Parameter::update(WebRequestWrapper* webRequestWrapper)
{
  String newValue = webRequestWrapper->arg(this->getId());
  this->update(newValue);
}
void Parameter::clearErrorMessage()
{
    this->errorMessage = NULL;
}

///////////////////////////////////////////////////////////////////////////////

TextParameter::TextParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : Parameter(label, id, valueBuffer, length, defaultValue)
{
  this->placeholder = placeholder;
  this->customHtml = customHtml;
}

void TextParameter::renderHtml(
  boolean dataArrived, WebRequestWrapper* webRequestWrapper)
{
  String content = this->renderHtml(
    dataArrived,
    webRequestWrapper->hasArg(this->getId()),
    webRequestWrapper->arg(this->getId()));
  webRequestWrapper->sendContent(content);
}
String TextParameter::renderHtml(
  boolean dataArrived, boolean hasValueFromPost, String valueFromPost)
{
  return this->renderHtml("text", hasValueFromPost, valueFromPost);
}
String TextParameter::renderHtml(
  const char* type, boolean hasValueFromPost, String valueFromPost)
{
  TextParameter* current = this;
  char parLength[5];

  String pitem = FPSTR(IOTWEBCONF_HTML_FORM_PARAM);

  pitem.replace("{b}", current->label);
  pitem.replace("{t}", type);
  pitem.replace("{i}", current->getId());
  pitem.replace("{p}", current->placeholder == NULL ? "" : current->placeholder);
  snprintf(parLength, 5, "%d", current->getLength());
  pitem.replace("{l}", parLength);
  if (hasValueFromPost)
  {
    // -- Value from previous submit
    pitem.replace("{v}", valueFromPost);
  }
  else
  {
    // -- Value from config
    pitem.replace("{v}", current->valueBuffer);
  }
  pitem.replace(
      "{c}", current->customHtml == NULL ? "" : current->customHtml);
  pitem.replace(
      "{s}",
      current->errorMessage == NULL ? "" : "de"); // Div style class.
  pitem.replace(
      "{e}",
      current->errorMessage == NULL ? "" : current->errorMessage);

  return pitem;
}

void TextParameter::update(String newValue)
{
  newValue.toCharArray(this->valueBuffer, this->getLength());
}

void TextParameter::debugTo(Stream* out)
{
  Parameter* current = this;
  out->print("'");
  out->print(current->getId());
  out->print("' with value: '");
  out->print(current->valueBuffer);
  out->println("'");
}

///////////////////////////////////////////////////////////////////////////////

NumberParameter::NumberParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : TextParameter(label, id, valueBuffer, length, defaultValue,
  placeholder, customHtml)
{
}

String NumberParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  return TextParameter::renderHtml("number", hasValueFromPost, valueFromPost);
}

///////////////////////////////////////////////////////////////////////////////

PasswordParameter::PasswordParameter(
  const char* label, const char* id, char* valueBuffer, int length,
  const char* defaultValue,
  const char* placeholder,
  const char* customHtml)
  : TextParameter(label, id, valueBuffer, length, defaultValue,
  placeholder, customHtml)
{
}

String PasswordParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  return TextParameter::renderHtml("password", true, String(""));
}

void PasswordParameter::debugTo(Stream* out)
{
  Parameter* current = this;
  out->print("'");
  out->print(current->getId());
  out->print("' with value: ");
#ifdef IOTWEBCONF_DEBUG_PWD_TO_SERIAL
  out->print("'");
  out->print(current->valueBuffer);
  out->println("'");
#else
  out->println(F("<hidden>"));
#endif
}

void PasswordParameter::update(String newValue)
{
  Parameter* current = this;
//  char temp[IOTWEBCONF_PASSWORD_LEN];
  char temp[current->getLength()];
  newValue.toCharArray(temp, current->getLength());
  if (temp[0] != '\0')
  {
    // -- Value was set.
    strncpy(current->valueBuffer, temp, current->getLength());
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.print("Updated ");
#endif
  }
  else
  {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.println("Was not changed ");
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////

CheckboxParameter::CheckboxParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    boolean defaultValue)
  : TextParameter(label, id, valueBuffer, length, defaultValue ? "selected" : NULL,
  NULL, NULL)
{
}

String CheckboxParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  boolean checkSelected = false;
  if (dataArrived)
  {
    if (hasValueFromPost && valueFromPost.equals("selected"))
    {
      checkSelected = true;
    }
  }
  else
  {
    if (this->isChecked())
    {
      checkSelected = true;
    }
  }

  if (checkSelected)
  {
    this->customHtml = CheckboxParameter::_checkedStr;
  }
  else
  {
    this->customHtml = NULL;
  }
  
  
  return TextParameter::renderHtml("checkbox", true, "selected");
}

void CheckboxParameter::update(String newValue)
{
  return TextParameter::update(newValue);
}

///////////////////////////////////////////////////////////////////////////////

OptionsParameter::OptionsParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t nameLength,
    const char* defaultValue)
  : TextParameter(label, id, valueBuffer, length, defaultValue,
  NULL, NULL)
{
  this->_optionValues = optionValues;
  this->_optionNames = optionNames;
  this->_optionCount = optionCount;
  this->_nameLength = nameLength;
}

///////////////////////////////////////////////////////////////////////////////

SelectParameter::SelectParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t nameLength,
    const char* defaultValue)
  : OptionsParameter(label, id, valueBuffer, length, optionValues, optionNames,
  optionCount, nameLength, defaultValue)
{
}

String SelectParameter::renderHtml(
  boolean dataArrived,
  boolean hasValueFromPost, String valueFromPost)
{
  TextParameter* current = this;

  String options = "";

  for (size_t i=0; i<this->_optionCount; i++)
  {
    const char *optionValue = (this->_optionValues + (i*this->getLength()) );
    const char *optionName = (this->_optionNames + (i*this->_nameLength) );
    String oitem = FPSTR(IOTWEBCONF_HTML_FORM_OPTION);
    oitem.replace("{v}", optionValue);
//    if (sizeof(this->_optionNames) > i)
    {
      oitem.replace("{n}", optionName);
    }
//    else
//    {
//      oitem.replace("{n}", "?");
//    }
    if ((hasValueFromPost && (valueFromPost == optionValue)) ||
      (strncmp(current->valueBuffer, optionValue, this->getLength()) == 0))
    {
      // -- Value from previous submit
      oitem.replace("{s}", " selected");
    }
    else
    {
      // -- Value from config
      oitem.replace("{s}", "");
    }

    options += oitem;
  }

  String pitem = FPSTR(IOTWEBCONF_HTML_FORM_SELECT_PARAM);

  pitem.replace("{b}", current->label);
  pitem.replace("{i}", current->getId());
  pitem.replace(
      "{c}", current->customHtml == NULL ? "" : current->customHtml);
  pitem.replace(
      "{s}",
      current->errorMessage == NULL ? "" : "de"); // Div style class.
  pitem.replace(
      "{e}",
      current->errorMessage == NULL ? "" : current->errorMessage);
  pitem.replace("{o}", options);

  return pitem;
}

///////////////////////////////////////////////////////////////////////////////

PrefixStreamWrapper::PrefixStreamWrapper(
  Stream* originalStream,
  std::function<size_t(Stream* stream)> prefixWriter)
{
  this->_originalStream = originalStream;
  this->_prefixWriter = prefixWriter;
}
size_t PrefixStreamWrapper::write(uint8_t data)
{
  size_t sizeOut = checkNewLine();
  sizeOut += this->_originalStream->write(data);
  if (data == 10) // NewLine
  {
    this->_newLine = true;
  }
  return sizeOut;
}
size_t PrefixStreamWrapper::write(const uint8_t *buffer, size_t size)
{
  size_t sizeOut = checkNewLine();
  sizeOut += this->_originalStream->write(buffer, size);
  if (*(buffer + size-1) == 10) // Ends with new line
  {
    this->_newLine = true;
  }
  return sizeOut;
}
size_t PrefixStreamWrapper::checkNewLine()
{
  if (this->_newLine)
  {
    this->_newLine = false;
    return this->_prefixWriter(this->_originalStream);
  }
  return 0;
}

} // end namespace