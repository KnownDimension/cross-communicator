#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>

#include <concord/discord.h>

u64snowflake Appid;
int i = 0;


void on_ready(struct discord *client, const struct discord_ready * event) {

    Appid = event->application->id;

    struct discord_application_command_option channels[] = {
        {
            .type = DISCORD_APPLICATION_OPTION_CHANNEL,
            .name = "channel select",
            .description = "select the channel you want the bot to be active in",
            .required = true,
            .channel_types = DISCORD_CHANNEL_GUILD_TEXT,
        },



    };

    struct discord_create_guild_application_command channelConfig = {
        .name = "channel configure",
        .description = "configures channel to use for cross communication",
        .default_permission = true,
        .options =
            &(struct discord_application_command_options){
                .size = sizeof(channels) / sizeof * channels, // divides the bite size of the entire array by one element in bytes of that array
                .array = channels,
            },
    };

    for (i = 0; i < event->guilds->size; i++ ) {
        u64snowflake guild = event->guilds->array[i].id;
        discord_create_guild_application_command(client, Appid, guild, &channelConfig, NULL); // used to create a slash command for each guild in an array of guilds
    };



}









void on_interaction_create(struct discord * client, const struct discord_interaction * event) {

        /* We're only interested on slash commands */
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) return;
    /* Return in case user input is missing for some reason */
    if (!event->data || !event->data->options) return;  

    char * channelchosen = "tbd";

    if (event->data->name = "channel configure") {
        const char * sqlStatement = "INSERT INTO channel_select VALUES(?, ?);";
        sqlite3_prepare_v2(db, *sqlStatement, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, event->data->guild_id, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 1, event->data->options->channelConfig, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);


    }

}






int main () {

    sqlite3 *db;
    char * dbErrorMessage;
    sqlite3_stmt *stmt;

    int Database = sqlite3_open("data.db", &db);

    if (Database != SQLITE_OK) {
        fprintf(stderr, "cant open database: %s/n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return -2;
    } // opening the database and checking if its opened without errors, if error, print message and return


    if (access("config.json", F_OK) == -1) { // checks if config file even exists
        printf("config.json does not exist, pls make one");
        return -1;
    }

    const char * config_file;
    config_file = "config.json"; // confic file name

    struct discord * client = discord_config_init(config_file); //  initialises a client based on the config file

    ccord_global_init(); // initialises shared resources

    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction_create);



    discord_run(client); // run


    sqlite3_close(db)
    discord_cleanup(client); // cleanup when its all ended
    ccord_global_cleanup();




}
