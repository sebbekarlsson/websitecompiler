#include "Website.h"


Website::Website(std::string directory) {
    this->dir = directory;

    ResourceManager::load(this->dir + "/site.json");
    this->site = nlohmann::json::parse(ResourceManager::get(this->dir + "/site.json"));

    mkdir(
            this->site["title"].get<std::string>().c_str(),
            S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
         );

    ResourceManager::load("src/shards/html.html");
    ResourceManager::load("src/shards/style.css");
    ResourceManager::load(this->dir + "/" + this->site["font"].get<std::string>());

    this->title = this->site["title"].get<std::string>(); 
    this->html = ResourceManager::get("src/shards/html.html");
    this->css = ResourceManager::get("src/shards/style.css");
    this->font = nlohmann::json::parse(ResourceManager::get(this->dir + "/" + this->site["font"].get<std::string>()));

    std::vector<std::string> _pages = this->site["pages"].get<std::vector<std::string>>();
    for(auto i : _pages) {
        if (!ResourceManager::fileExists(this->dir + "/" + i)) { continue; }
        ResourceManager::load(this->dir + "/" + i);
        if (
                ResourceManager::get(this->dir + "/" + i).empty() ||
                ResourceManager::get(this->dir + "/" + i) == ""
        ) { continue; }
        this->pages[
            this->replace_word(i, ".json", "")
        ] = ResourceManager::get(this->dir + "/" + i);
    }

    this->setFont(
            this->font["family"].get<std::string>(),
            this->font["fallback-family"].get<std::string>(),
            this->font["link"].get<std::string>()
            );
}

void Website::addHeaderElement(std::string text) {
    this->header.push_back(text);
}

void Website::setFont(std::string family, std::string fallbackFamily, std::string link) {
    this->family = family;
    this->fallbackFamily = fallbackFamily;
    this->addHeaderElement(
            "<link rel='stylesheet' type='text/css' href='"\
            + link + "'>"
            );
    this->css = this->replace_word(this->css, "{{family}}", this->family);
    this->css = this->replace_word(this->css, "{{fallback-family}}", this->fallbackFamily);
}

bool Website::addPage(std::string title, std::string content) {
    this->pages.insert(std::pair<std::string, std::string>(title, content));
}

void Website::generatePages() {
    for(auto const &ent1 : pages) {
        //std::cout << ent1.first << std::endl;
        //std::cout << pages.at(ent1.first) << std::endl;
        

        nlohmann::json page_json = nlohmann::json::parse(pages.at(ent1.first));
        ResourceManager::write_new(this->title + "/" + ent1.first + ".html",
                this->formatHTML(this->html)
                );
    }
}

void Website::generateCSS() {
    this->addHeaderElement(
            "<link rel='stylesheet' type='text/css' href='style.css'>"
            );

    ResourceManager::write_new(
            this->site["title"].get<std::string>() + "/style.css",
            this->css
            );
}

std::string Website::formatHTML(std::string _html) {
    std::string header_final = "";
    for(auto i : header) {
        header_final += i + "\n"; 
    }

    return this->replace_word(_html, "{{head}}", header_final);
}

std::string Website::replace_word(std::string text, std::string word, std::string replacement) {
    size_t start_pos = text.find(word);
    if(start_pos == std::string::npos)
        return text;
    text.replace(start_pos, word.length(), replacement);

    return text;
}

void Website::compile() {
    this->generateCSS();
    this->generatePages();
}
