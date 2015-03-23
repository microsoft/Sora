using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.IO;

namespace WixGen
{
    class WixCreator
    {
        public void CreateWix(string strConfigFile, string strWixFile)
        {
            try
            {
                System.Xml.XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(strConfigFile);
                XmlNamespaceManager nsmgr = new XmlNamespaceManager(xmlDoc.NameTable);
                nsmgr.AddNamespace("wix", "http://schemas.microsoft.com/wix/2006/wi");

                XmlNodeList nodeList = xmlDoc.DocumentElement.SelectNodes("//wix:FileGroup", nsmgr);
                int nId = 0;
                for (int i = 0; i < nodeList.Count; i++)
                {
                    XmlNode node = nodeList.Item(i);
                    XmlNode nodeParent = node.ParentNode;
                    string SrcDir = node.Attributes["SrcDir"].Value;
                    string Pattern = node.Attributes["Pattern"].Value;
                    nodeParent.RemoveChild(node);
                    string[] rgFiles = Directory.GetFiles(SrcDir, Pattern);
                    for (int j = 0; j < rgFiles.Length; j++)
                    {
                        //<File Id='Manual' Name='Manual.pdf' DiskId='1' Source='Manual.pdf'>
                        XmlElement elem = xmlDoc.CreateElement("File", "http://schemas.microsoft.com/wix/2006/wi");
                        
                        XmlAttribute attribId = xmlDoc.CreateAttribute("Id");
                        attribId.Value = string.Format("idFile_{0}", nId);
                        elem.Attributes.Append(attribId);

                        XmlAttribute attribName = xmlDoc.CreateAttribute("Name");
                        attribName.Value = System.IO.Path.GetFileName(rgFiles[j]);
                        elem.Attributes.Append(attribName);

                        XmlAttribute attribDiskId = xmlDoc.CreateAttribute("DiskId");
                        attribDiskId.Value = "1";
                        elem.Attributes.Append(attribDiskId);

                        XmlAttribute attribSource = xmlDoc.CreateAttribute("Source");
                        attribSource.Value = rgFiles[j];
                        elem.Attributes.Append(attribSource);

                        nodeParent.AppendChild(elem);
                        nId++;
                    }
                }

                nodeList = xmlDoc.DocumentElement.SelectNodes("//wix:FileRef", nsmgr);
                for (int i = 0; i < nodeList.Count; i++)
                {
                    XmlNode node = nodeList.Item(i);
                    XmlNode nodeParent = node.ParentNode;
                    string SrcName = node.Attributes["SrcName"].Value;
                    string DstName;
                    if (node.Attributes["DstName"] == null)
                    {
                        DstName = "";
                    }
                    else
                    {
                        DstName = node.Attributes["DstName"].Value;
                    }
                    nodeParent.RemoveChild(node);

                    XmlElement elem = xmlDoc.CreateElement("File", "http://schemas.microsoft.com/wix/2006/wi");

                    XmlAttribute attribId = xmlDoc.CreateAttribute("Id");
                    attribId.Value = string.Format("idFile_{0}", nId);
                    elem.Attributes.Append(attribId);

                    XmlAttribute attribName = xmlDoc.CreateAttribute("Name");
                    if (DstName.Equals(""))
                    {
                        attribName.Value = System.IO.Path.GetFileName(SrcName);
                    }
                    else
                    {
                        attribName.Value = DstName;
                    }
                    elem.Attributes.Append(attribName);

                    XmlAttribute attribDiskId = xmlDoc.CreateAttribute("DiskId");
                    attribDiskId.Value = "1";
                    elem.Attributes.Append(attribDiskId);

                    XmlAttribute attribSource = xmlDoc.CreateAttribute("Source");
                    attribSource.Value = SrcName;
                    elem.Attributes.Append(attribSource);

                    nodeParent.AppendChild(elem);
                    nId++;
                }

                nodeList = xmlDoc.DocumentElement.SelectNodes("//wix:Component", nsmgr);
                for (int i = 0; i < nodeList.Count; i++)
                {
                    XmlNode node = nodeList.Item(i);
                    string guid = node.Attributes["Guid"].Value;
                    if (guid.Equals("*"))
                    {
                        node.Attributes["Guid"].Value = System.Guid.NewGuid().ToString();
                    }
                }

                //XmlWriter xw = new XmlTextWriter(strWixFile, Encoding.UTF8);
                xmlDoc.Save(strWixFile);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
            }
        }
    }
}
